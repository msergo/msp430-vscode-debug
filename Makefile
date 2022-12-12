OBJECTS=main.o
MAP=main.map
MAKEFILE=Makefile

ifeq ($(OS),Windows_NT)
	ifeq ($(shell uname -o),Cygwin)
		RM= rm -rf
	else
		RM= del /q
	endif
else
	RM= rm -rf
endif

GCC_DIR = /opt/ti/msp430-gcc/bin/
SUPPORT_FILE_DIRECTORY = /opt/ti/msp430-gcc/include

DEVICE  = MSP430F5529
CC      = $(GCC_DIR)/msp430-elf-gcc
GDB     = $(GCC_DIR)/msp430-elf-gdb

CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -O0 -Wall -g3 -gdwarf-2 -ggdb
LFLAGS = -L $(SUPPORT_FILE_DIRECTORY) -Wl,-Map,$(MAP),--gc-sections 

all: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out

clean: 
	$(RM) $(OBJECTS)
	$(RM) $(MAP)
	$(RM) *.out
	
debug: all
	$(GDB) $(DEVICE).out
