#
# Makefile.  But you knew that.
#

COMPILER=GCC
SUBARCH=ARM_CM3
PROC=LM3S8962
BOARD=LM3S8962_EVB

# Where we get pieces from...
SRC_DIR=src
FREERTOS=../FreeRTOS
STELLARISWARE=../StellarisWare-ek-lm3s-8962

RTOS_SOURCE_DIR=$(FREERTOS)/Source
RTOS_COMMON_DIR=$(FREERTOS)/Common/Minimal
RTOS_INCLUDE_DIR=$(FREERTOS)/Common/include
UIP_COMMON_DIR=$(FREERTOS)/Common/ethernet/uIP/uip-1.0/uip
LUMINARY_DRIVER_LIB=$(STELLARISWARE)/driverlib/gcc

# Misc. executables.
RM=/bin/rm
DOXYGEN=/usr/bin/doxygen

include makedefs

PROG=LM3S8962_EVB
LDSCRIPT=standalone.ld

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

CPPFLAGS +=\
	-I $(SRC_DIR) \
	-I $(RTOS_SOURCE_DIR)/include \
	-I $(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3 \
	-I $(RTOS_INCLUDE_DIR) \
	-I $(UIP_COMMON_DIR) \
	-I $(STELLARISWARE) \
	-I $(STELLARISWARE)/inc \
	-I $(STELLARISWARE)/utils \
	-I $(STELLARISWARE)/driverlib \
	-I $(STELLARISWARE)/boards/ek-lm3s8962 \
	-I $(SRC_DIR)/webserver \
	-D $(COMPILER)_$(SUBARCH) \
	-D inline= \
	-D PACK_STRUCT_END=__attribute\(\(packed\)\) \
	-D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
	-D sprintf=usprintf -D snprintf=usnprintf \
	-D vsnprintf=uvsnprintf -D printf=uipprintf \
	-D $(BOARD) \
	-D DEPRECATED

CFLAGS +=\
	$(CPPFLAGS) \
	$(DEBUG) $(OPTIM) \
	-Wall

VPATH = $(SRC_DIR) $(SRC_DIR)/webserver \
	$(STELLARISWARE)/utils \
	$(STELLARISWARE)/boards/ek-lm3s8962/drivers \
	$(UIP_COMMON_DIR) \
	$(RTOS_SOURCE_DIR) \
	$(RTOS_SOURCE_DIR)/portable/$(COMPILER)/$(SUBARCH) \
	$(RTOS_SOURCE_DIR)/portable/MemMang

SOURCE =\
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/io.c \
	$(SRC_DIR)/util.c \
	$(SRC_DIR)/partnum.c \
	$(SRC_DIR)/logger.c \
	$(SRC_DIR)/timertest.c \
	$(SRC_DIR)/webserver/uIP_Task.c \
	$(SRC_DIR)/webserver/emac.c \
	$(SRC_DIR)/webserver/httpd.c \
	$(SRC_DIR)/webserver/httpd-cgi.c \
	$(SRC_DIR)/webserver/httpd-fs.c \
	$(SRC_DIR)/webserver/http-strings.c \
	$(STELLARISWARE)/utils/ustdlib.c \
	$(STELLARISWARE)/boards/ek-lm3s8962/drivers/rit128x96x4.c \
	$(UIP_COMMON_DIR)/uip_arp.c \
	$(UIP_COMMON_DIR)/psock.c \
	$(UIP_COMMON_DIR)/timer.c \
	$(UIP_COMMON_DIR)/uip.c \
	$(RTOS_SOURCE_DIR)/list.c \
	$(RTOS_SOURCE_DIR)/queue.c \
	$(RTOS_SOURCE_DIR)/tasks.c \
	$(RTOS_SOURCE_DIR)/portable/$(COMPILER)/$(SUBARCH)/port.c \
	$(RTOS_SOURCE_DIR)/portable/MemMang/heap_2.c

LIBS= $(LUMINARY_DRIVER_LIB)/libdriver.a

OBJS = $(addprefix $(BUILD_DIR), $(notdir $(SOURCE:.c=.o)))

.PHONY: all doxygen clean distclean webfiles webstrings

all: $(BUILD_DIR)$(PROG).bin

# Include the dependencies if they are available

-include $(wildcard $(BUILD_DIR)*.d) __dummy__


$(BUILD_DIR)$(PROG).bin : $(BUILD_DIR)$(PROG).axf

$(BUILD_DIR)$(PROG).axf : $(BUILD_DIR)startup.o \
	webfiles webstrings \
	$(OBJS) $(LIBS)

$(BUILD_DIR)startup.o : startup.c Makefile
	@echo "  $(CC) $<"
	$(Q)$(CC) -O1 $(filter-out -O%, $(CFLAGS)) -o $@ $<

# Phony targets to auto-generate the .c files for the webserver
# This isn't right, causes rebuild every time.  Grrrstupid.
webfiles:
	(cd $(SRC_DIR)/webserver && ./makefsdata)

webstrings:
	(cd $(SRC_DIR)/webserver && ./makestrings)

#$(SRC_DIR)/webserver/httpd-fsdata.c : \
#		$(wildcard ($SRC_DIR)/webserver/httpd-fs/*)
#	(cd $(SRC_DIR)/webserver && ./makefsdata)
#
#$(SRC_DIR)/webserver/http-strings.c : $(SRC_DIR)/webserver/http-strings
#	(cd $(SRC_DIR)/webserver && ./makestrings)

doxygen :
	$(DOXYGEN) doxygen.cfg

clean :
	#touch Makefile
	$(RM) -f $(BUILD_DIR)*.[od]
	rm -f $(SRC_DIR)/webserver/httpd-fsdata.c
	rm -f $(SRC_DIR)/webserver/webserver/http-strings.[ch]
	rm -rf doc

distclean : clean
	$(RM) -f $(BUILD_DIR)*.axf $(BUILD_DIR)*.bin $(BUILD_DIR)*.map
	find . -name "*.~" -exec rm -f {} \;
	find . -name "*.o" -exec rm -f {} \;
