/**
 * \file io.c
 *
 * I/O implementation.

\page iopage1 I/O Overview

\addindex I/O Task

The I/O support subsystem has one task, io_task(), which is used to
periodically read the I/O points (analog and discretes).  This is done
so that the program can read the I/O without blocking.  The problem with
blocking until the I/O is complete is that this is a "real time" system
and blocking will disrupt the "real time" attribute.
- The trade-off is that the value will be "old" by up to one scan period.
- If the io_task() poll rate is increased, it will keep the data
    "fresher", but will result in more overhead.

Processor-based discrete I/O (GPIO) can be read without any delay,
so that is an exception and is read directly.

Analog I/O requires a conversion sequence which takes time to complete,
so that is done in the polling task.  Other I/O, e.g. I2C- or SPI-attached
chips, also require time to perform the read/write sequence and thus are
done in the I/O task.

 *
 * \addtogroup io I/O
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7        
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdint.h>
#include <string.h>

#include <hw_types.h>
#include <hw_memmap.h>
#include <hw_adc.h>
#include <sysctl.h>
#include <adc.h>
#include <ssi.h>
#include <gpio.h>

#include <config.h>
#include <io.h>
#include <util.h>
#include <logger.h>

#define DEBUG	1

/*
 * Public information.
 */
xTaskHandle io_task_handle;	/**< I/O task handle */

/*
 * Private information.
 */
#define POLL_HZ		10
#define POLL_DELAY	(configTICK_RATE_HZ / POLL_HZ)

#define IO_TIMEOUT	(POLL_DELAY / 10)

static void io_task(void *params);

/** Mutex for exclusive access to the data. */
static xSemaphoreHandle io_mutex;

/*
 * Internal A/D converter.
 *
 * WARNING:
 * + The adc_val array *must* be unsigned long because library
 *     call requires this (passed in as a pointer).
 * + We define the array to be 8 samples because the hardware *can* do
 *     8 samples and could return that many, overwriting other variables
 *     if we don't have the space.
 */
#define ADC_SAMPLES		5	/* We use 5 processor channels */
#define PROC_ADC_CHANNELS	8	/* Need to reserve room for all */

static volatile unsigned long adc_val[PROC_ADC_CHANNELS];


/****************************************************************************/

/**
 * Translate ADC strings to the adc enum.
 */
const struct enumxlate_s adcxlate[] = {
	{"adcProc0", sizeof("adcProc0") - 1},
	{"adcProc1", sizeof("adcProc1") - 1},
	{"adcProc2", sizeof("adcProc2") - 1},
	{"adcProc3", sizeof("adcProc3") - 1},
	{"adcProcTemp", sizeof("adcProcTemp") - 1},
	{"adcUnused1", sizeof("adcUnused1") - 1},
	{"adcUnused2", sizeof("adcUnused2") - 1},
	{"adcUnused3", sizeof("adcUnused3") - 1},
	{"adcInvalid", sizeof("adcInvalid") - 1},
};

const char *adctostr(enum adc_sel which)
{
	if ((which < adcProc0) || (which > adcInvalid))
		return adcxlate[adcInvalid].str;
	return adcxlate[which].str;
}

enum adc_sel strtoadc(const char *which)
{
	int j;

	if ((which[0] != 'a') || (which[1] != 'd') || (which[2] != 'c'))
		return adcInvalid;
	for (j = 0; j < adcInvalid; j++) {
		if (strncmp(which, adcxlate[j].str, adcxlate[j].len) == 0)
			return j;
	}
	return adcInvalid;
}

/****************************************************************************/

/**
 * Translate DIO strings to the dio enum.
 */
const struct enumxlate_s dioxlate[] = {
	/*
	 * Processor discretes.
	 */
	{"dioUp", sizeof("dioUp") - 1},
	{"dioDown", sizeof("dioDown") - 1},
	{"dioLeft", sizeof("dioLeft") - 1},
	{"dioRight", sizeof("dioRight") - 1},
	{"dioSelect", sizeof("dioSelect") - 1},
	{"dioLed0", sizeof("dioLed0") - 1},

	{"dioInvalid", sizeof("dioInvalid") - 1}
};

const char *diotostr(enum dio_sel which)
{
	if ((which < dioUp) || (which > dioInvalid))
		return dioxlate[dioInvalid].str;
	return dioxlate[which].str;
}

enum dio_sel strtodio(const char *which)
{
	int j;

	if ((which[0] != 'd') || (which[1] != 'i') || (which[2] != 'o'))
		return dioInvalid;
	for (j = 0; j < dioInvalid; j++) {
		if (strncmp(which, dioxlate[j].str, dioxlate[j].len) == 0)
			return j;
	}
	return dioInvalid;
}

/****************************************************************************/

/*
 * Get a discrete I/O value.
 */
int dio(enum dio_sel which)
{
	/*
	 * Note: This does not use the mutex to lock the I/O because it
	 * is single bit (byte) reads that will be inherently atomic.
	 */

	/*
	 * Processor discretes.
	 */
	switch (which) {
	case dioUp:
		return GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0) ? 1 : 0;
	case dioDown:
		return GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1) ? 1 : 0;
	case dioLeft:
		return GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2) ? 1 : 0;
	case dioRight:
		return GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3) ? 1 : 0;
	case dioSelect:
		return GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ? 1 : 0;
	case dioLed0:
		return GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) ? 1 : 0;
	default:
		return -1;	/* Shouldn't get here */
	}
	lprintf("dio(%d) - Unknown discrete I/O requested\r\n", which);
	return -1;	/* Shouldn't get here */
}

/****************************************************************************/

/*
 * Set/reset a discrete only if it has changed state.
 *
 * Returns the previous state.
 */
int dio_setif(enum dio_sel which, int value)
{
	if (dio(which) != value)
		return(dio_set(which, value));
	return value;
}

/****************************************************************************/

/*
 * Set a discrete I/O value.
 *
 * Returns the previous state or -1 if it is not setable.
 */
int dio_set(enum dio_sel which, int value)
{
	int ret   = -1;	/* default return: invalid */

	/*
	 * Protect the read/modify/write operation.
	 */
	if (xSemaphoreTake(io_mutex, IO_TIMEOUT) == pdTRUE) {
		switch (which) {
		case dioLed0:
			ret = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) ? 1 : 0;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,
				value ? GPIO_PIN_0 : 0);
			break;
		default:
			lprintf("dio_set(%d) - Invalid discrete.\r\n", which);
			break;
		}
		xSemaphoreGive(io_mutex);
	} else {
		lprintf("dio_set() semaphore timeout line %d\r\n", __LINE__);
	}
	return ret;
}

/****************************************************************************/

/*
 * Returns an integer representing the value based on the scaling:
 *   raw: A/D conversion value
 *   millivolts: A/D converted to millivolts (integer)
 *   engineering: A/D converted to engineering units * 1000 (i.e. milliunits)
 */
int adc(enum adc_sel which, enum adc_units scaling)
{
	unsigned long raw_val;	/* library defines ADC as unsigned long */

	if (xSemaphoreTake(io_mutex, IO_TIMEOUT) == pdTRUE) {
		raw_val = adc_val[which];
		xSemaphoreGive(io_mutex);
	} else {
		lprintf("I2C semaphore timeout line %d\r\n", __LINE__);
		return 0;
	}

	switch (scaling) {
	case raw:
		return raw_val;
	case millivolts:
		return (raw_val * VREF) / 1024;
	case engineering:
		if (which == adcProcTemp) {
			/*
			 * SENSOR = 2.7 - ((T + 55) / 75)
			 * T = ((2.7 - SENSOR) * 75) - 55
			 */
			return ((2700 - (raw_val * VREF) / 1024) * 75) - 55000;
		}
		/*
		 * We don't have engineering units, just return mV
		 */
		return (raw_val * VREF) / 1024;
	}
	return -1;	/* Shouldn't get here */
}

/****************************************************************************/

/**
 * Configure the processor's A/D converter subsystem.
 */
static void adc_setup(void)
{
	ADCHardwareOversampleConfigure(ADC0_BASE, ADC_SAC_AVG_OFF);
	ADCReferenceSet(ADC0_BASE, ADC_REF_INT);

	/*
	 * Create a sample sequence for our A/D (what inputs to sample).
	 */
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3);
	/* The last conversion is the internal temperature sensor */
	ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_TS |
		ADC_CTL_IE | ADC_CTL_END);
}

/****************************************************************************/

/**
 * Do a processor A/D conversion sequence.
 */
static void scan_proc_adc(void)
{
	int samples;

	/*
	 * We occasionally get too many or too few samples because
	 * the extra (missing) samples will show up on the next read
	 * operation.  Just do it again if this happens.
	 */
	for (samples = 0; samples != ADC_SAMPLES; ) {
		ADCSequenceEnable(ADC0_BASE, 0);
		ADCProcessorTrigger(ADC0_BASE, 0);
		/*
		 * Wait until the sample sequence has completed.
		 */
		while(!ADCIntStatus(ADC0_BASE, 0, false))
			;
		/*
		 * Read the values from the ADC.  The whole sequence
		 * gets converted and stored in one fell swoop.
		 */
		if (xSemaphoreTake(io_mutex, IO_TIMEOUT)) {
			samples = ADCSequenceDataGet(ADC0_BASE, 0,
				(unsigned long *)adc_val);
			xSemaphoreGive(io_mutex);
		}
#if (DEBUG > 0)
		if (samples != ADC_SAMPLES) {
			lprintf("A/D samples: is %d, "
				"should be %d.\r\n",
				samples, ADC_SAMPLES);
		}
#endif
	}
}

/****************************************************************************/

/**
 * I/O task: reads and writes the I/O.
 */
static void io_task(void *params)
{
	portTickType last_wake_time;
	int ticks=0;

	adc_setup();

#if (DEBUG > 0)
	lprintf("io_task() running.\r\n");
#endif

	/* Start our periodic time starting in 3. 2. 1. NOW! */
	last_wake_time = xTaskGetTickCount();

	while(1) {	/* forever loop */
		wdt_checkin[wdt_io] = 0;

		scan_proc_adc();

		/*
		 * Send a char out the serial port every 10 sec.
		 */
		if (ticks>=POLL_HZ*10) {
			lstr(".");
			ticks=0;
		}
		ticks++;

		vTaskDelayUntil(&last_wake_time, POLL_DELAY);
	}
}

/****************************************************************************/

/*
 * Initialize the I/O.
 */
int io_init(void)
{
	portBASE_TYPE ret;
	int j;

	for (j = 0; j < sizeof(adc_val) / sizeof(adc_val[0]); j++)
		adc_val[j] = 0;

	io_mutex = xSemaphoreCreateMutex();
	if (io_mutex == NULL)
		return -1;	/* return failure flag */

	ret = xTaskCreate(io_task,
		(signed portCHAR *)"io",
		DEFAULT_STACK_SIZE,
		NULL,
		IO_TASK_PRIORITY,
		&io_task_handle);
	if (ret != pdPASS)
		lprintf("Creation of IO task failed: %d\r\n", ret);

	return 0;
}
/** \} */
