CC=gcc
CFLAGS=-pthread -lrt -DM5OP_ADDR=0xFFFF0000
SFLAGS= -DM5OP_ADDR=0xFFFF0000
# Final run:
CFLAGS += -O2

OBJ           ?= .o

OBJOPT        ?= -c -o $@
# or, if you want debugging:
# CFLAGS += -ggdb -fno-inline -O0 -DCAST_DEBUG

all: spec_cast
clean:
	rm -f spec_cast.o m5_mapMem_c.o spec_cast m5op_x86_c.o

# C
%$(OBJ): %.c
	$(CC) $(OBJOPT) $(CFLAGS) $<

# S
%$(OBJ): %.S
	$(CC) $(OBJOPT) $(SFLAGS) $<

spec_cast: spec_cast.o m5_mapMem_c.o m5op_x86_c.o
	${CC} ${CFLAGS} -o spec_cast spec_cast.o m5_mapMem_c.o m5op_x86_c.o
