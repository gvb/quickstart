#
# Copyright (c) 2011 Gerald Van Baren
# All rights reserved. 
# 
# Redistribution and use in source and binary forms, with or without
# modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

COMPILER=GCC
SUBARCH=ARM_CM3
#PROC=LM3S8962
BOARD=LM3S8962_EVB

# Where we get pieces from...
SRC_DIR=src
FREERTOS=../FreeRTOS
STELLARISWARE=../StellarisWare
LWIP=../lwip
LWIP_CONTRIB=../lwip-contrib/ports/cross

RTOS_SOURCE_DIR=$(FREERTOS)/Source
RTOS_COMMON_DIR=$(FREERTOS)/Common/Minimal
RTOS_INCLUDE_DIR=$(FREERTOS)/Common/include
LUMINARY_DRIVER_LIB=$(STELLARISWARE)/driverlib/gcc
LWIP_INCLUDE_DIR=$(LWIP)/src/include

ifndef WDT_ENABLE
WDT_ENABLE=1
endif

CROSS_COMPILE = arm-none-eabi-

# Misc. executables.
RM=/bin/rm
DOXYGEN=/usr/bin/doxygen

PROG=LM3S8962_EVB
LDSCRIPT=standalone.ld


#
# The flags passed to the assembler.
#
AFLAGS =	-mthumb \
		-mcpu=cortex-m3 \
		-MD

#
# The flags passed to the compiler.
#
CFLAGS =	-mthumb \
		-mcpu=cortex-m3 \
		-Os \
		-ffunction-sections \
		-fdata-sections \
		-std=c99 \
		-Wall \
		-pedantic \
		-DPART_$(PART) \
		-c

# Build for debug
CFLAGS +=	-g

#
# The flags passed to the linker.
#
LDFLAGS =\
		-nostartfiles \
		-Map=$(BUILD_DIR)/$(PROG).map \
		--no-gc-sections

# should use --gc-sections but the debugger does not seem to be able to
# cope with the option.
LINKER_FLAGS +=\
	-nostartfiles \
	-Xlinker \
	-M \
	-Xlinker \
	-Map=$(BUILD_DIR)$(PROG).map \
	-Xlinker \
	--no-gc-sections

#DEBUG=-g
OPTIM=-Os


#TBD do we need these in CPPFLAGS?:
#	-D PACK_STRUCT_END=__attribute\(\(packed\)\) \
#	-I $(RTOS_INCLUDE_DIR) \

CPPFLAGS +=\
	-I $(SRC_DIR)/app-opts \
	-I $(SRC_DIR)/quick-opts \
	-I $(SRC_DIR)/app \
	-I $(SRC_DIR)/quick \
	-I $(RTOS_SOURCE_DIR)/include \
	-I $(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3 \
	-I $(STELLARISWARE) \
	-I $(STELLARISWARE)/inc \
	-I $(STELLARISWARE)/utils \
	-I $(STELLARISWARE)/driverlib \
	-I $(STELLARISWARE)/boards/ek-lm3s8962 \
	-I $(SRC_DIR)/webserver \
	-I $(LWIP_INCLUDE_DIR) \
	-I $(LWIP_INCLUDE_DIR)/ipv4 \
	-I $(LWIP_CONTRIB)/src/include \
	-I $(LWIP_CONTRIB)/src/include/LM3S \
	-I $(STELLARISWARE)/third_party \
	-D $(COMPILER)_$(SUBARCH) \
	-D inline= \
	-D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
	-D sprintf=usprintf -D snprintf=usnprintf \
	-D vsnprintf=uvsnprintf -D printf=uipprintf \
	-D $(BOARD) \
	-D DEPRECATED \
	-D WDT_ENABLE=$(WDT_ENABLE)\
	$(SET_IP_ADR) $(PROTECT_PERMCFG)

CFLAGS +=\
	$(CPPFLAGS) \
	$(DEBUG) $(OPTIM) \
	-Wall

WEBSOURCE=$(wildcard $(SRC_DIR)/httpd-fs)

APPSOURCE=

-include $(SRC_DIR)/app/makeapp

SOURCE =\
	$(APPSOURCE) \
	$(SRC_DIR)/quick/main.c \
	$(SRC_DIR)/quick/io.c \
	$(SRC_DIR)/quick/util.c \
	$(SRC_DIR)/quick/partnum.c \
	$(SRC_DIR)/quick/logger.c \
	$(SRC_DIR)/quick/timertest.c \
	$(SRC_DIR)/quick/ETHIsr.c \
	$(SRC_DIR)/quick/LWIPStack.c \
	$(SRC_DIR)/quick/fs.c \
	$(SRC_DIR)/quick/httpd.c \
	$(SRC_DIR)/quick/httpd-cgi.c \
	$(SRC_DIR)/quick/debugSupport.c \
	$(STELLARISWARE)/utils/ustdlib.c \
	$(STELLARISWARE)/boards/ek-lm3s8962/drivers/rit128x96x4.c \
	$(RTOS_SOURCE_DIR)/list.c \
	$(RTOS_SOURCE_DIR)/queue.c \
	$(RTOS_SOURCE_DIR)/tasks.c \
	$(RTOS_SOURCE_DIR)/portable/$(COMPILER)/$(SUBARCH)/port.c \
	$(RTOS_SOURCE_DIR)/portable/MemMang/heap_2.c \
	$(BUILD_DIR)buildDate.c

VPATH	= $(sort $(dir $(SOURCE)))

LIBS= $(LUMINARY_DRIVER_LIB)/libdriver.a $(LWIP_CONTRIB)/liblwip.a

OBJS = $(addprefix $(BUILD_DIR), $(notdir $(SOURCE:.c=.o)))

#TBD Talk to Jerry why this needs to be here...

include makedefs

.PHONY: all doxygen clean distclean get-date

all: $(BUILD_DIR)$(PROG).bin

# Include the dependencies if they are available

-include $(BUILD_DIR)depend

get-date:
	echo "const char buildDate[]=\"`date -R`\";" > $(BUILD_DIR)buildDate.c

$(BUILD_DIR)buildDate.c : get-date

$(BUILD_DIR)$(PROG).bin : $(BUILD_DIR)$(PROG).axf

$(BUILD_DIR)$(PROG).axf : $(BUILD_DIR)startup.o $(OBJS) $(LIBS)

$(BUILD_DIR)startup.o : startup.c Makefile
	@echo "  $(CC) $<"
	$(Q)$(CC) -O1 $(filter-out -O%, $(CFLAGS)) -o $@ $<

$(SRC_DIR)/quick/fs.c : $(BUILD_DIR)fsdata.c $(BUILD_DIR)fsdata-stats.c

$(BUILD_DIR)fsdata.c $(BUILD_DIR)fsdata-stats.c : $(WEBSOURCE)
	@echo "./makefsdata"
	$(Q)./makefsdata $(SRC_DIR)/httpd-fs $(BUILD_DIR)fsdata.c $(BUILD_DIR)fsdata-stats.c

$(BUILD_DIR)depend: $(SOURCE)
	@echo "  generate $(BUILD_DIR)depend -> $(CC) $<"
	$(Q)$(CC) $(CPPFLAGS) -MM $(addprefix -MT, $(OBJS)) -E $^ > $@ || rm -f $(BUILD_DIR)depend

doxygen :
	$(DOXYGEN) doxygen.cfg

clean :
	$(RM) -f $(BUILD_DIR)*.[od]
	$(RM) -f $(BUILD_DIR)*.map
	$(RM) -f $(BUILD_DIR)*.axf	
	$(RM) -f $(BUILD_DIR)*.bin
	$(RM) -f $(BUILD_DIR)depend
	$(RM) -f $(BUILD_DIR)*.c

distclean : clean
	$(RM) -f $(BUILD_DIR)*.axf $(BUILD_DIR)*.bin $(BUILD_DIR)*.map
	find . -name "*.~" -exec rm -f {} \;
	find . -name "*.o" -exec rm -f {} \;

