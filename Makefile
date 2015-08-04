MPICC = mpicc
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LFLAGS = 
LIBS = -lgsl -lgslcblas -lm -lconfuse
INC_DIR = include/
SRC_DIR = src/
INCLUDES = $(addprefix -I,$(INC_DIR))
INCLUDES += $(addprefix -I,$(SRC_DIR))
SOURCES = emeans.c io.c
OBJECTS = $(subst .c,.o,$(SOURCES))
EXE = emeans.exe
.PHONY: clean help

.PHONY: debug  
debug: CFLAGS += -O0 -g3 -DDEBUG_ALL 
debug: $(EXE)

.PHONY: release  
release: CFLAGS += -O2 -march=native
release: $(EXE) cleanup

emeans.exe : emeans.o io.o
	$(MPICC) $(INCLUDES) $(CFLAGS) $^ $(LIBS) -o $@ 

%.o : $(SRC_DIR)%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< 

all : $(EXE)

clean:
	rm -f $(OBJECTS) $(EXE) *~

cleanup:
	rm -f $(OBJECTS) *~

help:
	@echo "Valid targets:"
	@echo "  all:    generates all binary files"
	@echo "  clean:  removes .o and .exe files"
