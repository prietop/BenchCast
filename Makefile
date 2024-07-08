CC=gcc
CFLAGS=
SFLAGS= 
OBJ = bench_cast.o 
ifdef GEM5
CFLAGS += -DM5OP_ADDR=0xFFFF0000
SFLAGS += -DM5OP_ADDR=0xFFFF0000
OBJ += m5_mmap.o m5op_addr.o
endif
# Final run:
CFLAGS += -O2
LDFLAGS += -pthread -lrt

ifdef PAPI
CFLAGS += -DPAPI
LDFLAGS += -lpapi
#LDFLAGS += /usr/local/lib/libpapi.a
endif

OBJOPT        ?= -c -o $@
# or, if you want debugging:
# CFLAGS += -ggdb -fno-inline -O0 -DCAST_DEBUG

all: bench_cast
clean:
	rm -f ${OBJ} bench_cast

# C
%.o: %.c
	$(CC) $(OBJOPT) $(CFLAGS) $<

# S
%.o: %.S
	$(CC) $(OBJOPT) $(SFLAGS) $<

bench_cast: ${OBJ}
	${CC} ${CFLAGS} -o bench_cast ${OBJ} ${LDFLAGS}
