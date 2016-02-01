CC=gcc
CFLAGS=-lpthread -lrt -DM5OP_ADDR=0xFFFF0000
SFLAGS= -DM5OP_ADDR=0xFFFF0000
# Final run:
CFLAGS += -O2

OBJ           ?= .o

OBJOPT        ?= -c -o $@
# or, if you want debugging:
# CFLAGS += -ggdb -fno-inline -O0 -DCAST_DEBUG

all: gem5_spec_cast
clean:
	rm -f gem5_spec_cast.o m5_mapMem_c.o gem5_spec_cast m5op_x86_c.o

# C
%$(OBJ): %.c
	$(CC) $(OBJOPT) $(CFLAGS) $<

# S
%$(OBJ): %.S
	$(CC) $(OBJOPT) $(SFLAGS) $<

gem5_spec_cast: gem5_spec_cast.o m5_mapMem_c.o m5op_x86_c.o
	${CC} ${CFLAGS} -o gem5_spec_cast gem5_spec_cast.o m5_mapMem_c.o m5op_x86_c.o
