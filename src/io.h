/**
 * \file io.h
 *
 * I/O definitions and declarations.
 *
 * \addtogroup io I/O
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7        
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

#ifndef IO_H_
#define IO_H_

/**
 * Structure used to translate enums to strings and vice versa.
 */
struct enumxlate_s {
        const char *str;	/**< The enum item as a string */
        const int  len;		/**< String length, excluding the null */
};

/****************************************************************************/

/**
 * Selects the analog I/O format.
 */
enum adc_units {
	raw,			/**< Raw (ADC counts) */
	millivolts,		/**< Millivolts measured by the ADC */
	engineering,		/**< Engineering value, in milliunits */
};

/**
 * Selects the A/D conversion channel.
 */
enum adc_sel {
	adcProc0,		/**< Processor ADC */
	adcProc1,		/**< Processor ADC */
	adcProc2,		/**< Processor ADC */
	adcProc3,		/**< Processor ADC */
	adcProcTemp,		/**< On-chip temperature sensor */
	adcUnused1,		/**< Unused */
	adcUnused2,		/**< Unused */
	adcUnused3,		/**< Unused */
	adcInvalid		/**< Invalid flag, MUST BE LAST */
};

extern const struct enumxlate_s adcxlate[];	/* for static translations */

/**
 * ADC enum to string representation.
 */
const char *adctostr(enum adc_sel which);

/**
 * String representation to ADC enum.
 */
enum adc_sel strtoadc(const char *which);

/****************************************************************************/

/**
 * Selects the discrete I/O.
 */
enum dio_sel {
	/*
	 * Processor discretes.
	 */
	dioUp,				/**< Up button */
	dioDown,			/**< Down button */
	dioLeft,			/**< Left button */
	dioRight,			/**< Right button */
	dioSelect,			/**< Select button */
	dioLed0,			/**< output: on-board LED */

	dioInvalid			/**< Invalid flag, MUST BE LAST */
};

/**
 * For static translations
 */
extern const struct enumxlate_s dioxlate[];

/**
 * Discrete I/O enum to string representation.
 */
const char *diotostr(enum dio_sel which);

/**
 * String representation to discrete I/O enum.
 */
enum dio_sel strtodio(const char *which);

/****************************************************************************/

/**
 * Initialize and kick off the I/O tasks.
 */
int io_init(void);

/**
 * Get a discrete I/O value.
 *
 * \param which Selects which discrete I/O point to read.
 * \returns the discrete I/O status {1|0}
 */
int dio(enum dio_sel which);

/**
 * Set/reset a discrete only if it has changed state.
 *
 * \param which Selects which discrete I/O point to read.
 * \returns The previous state {1|0} or -1 if it is not setable.
 */
int dio_setif(enum dio_sel which, int value);

/**
 * Set a discrete I/O value.
 *
 * \param which Selects which discrete I/O point to read.
 * \returns The previous state {1|0} or -1 if it is not setable.
 */
int dio_set(enum dio_sel which, int value);

/**
 * Returns an integer representing the value based on the scaling.
 *
 * \param which Selects which analog input to read.
 * \param scaling Selects the scaling of the analog input.
 *   - raw: A/D conversion value
 *   - millivolts: A/D converted to millivolts (integer)
 *   - engineering: A/D converted to engineering units * 1000 (i.e. milliunits)
 * \return The value of the analog input, scaled as requested.
 */
int adc(enum adc_sel which, enum adc_units scaling);

#endif
/** \} */
