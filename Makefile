CC=gcc
CFLAGS=
SFLAGS= 
OBJ = spec_cast.o 
ifdef GEM5
CFLAGS += -DM5OP_ADDR=0xFFFF0000
SFLAGS += -DM5OP_ADDR=0xFFFF0000
OBJ += m5_mapMem_c.o m5op_x86_c.o
endif
# Final run:
CFLAGS += -O2
LDFLAGS += -pthread -lrt /usr/local/lib/libpapi.a

OBJOPT        ?= -c -o $@
# or, if you want debugging:
# CFLAGS += -ggdb -fno-inline -O0 -DCAST_DEBUG

all: spec_cast
clean:
	rm -f ${OBJ} spec_cast

# C
%.o: %.c
	$(CC) $(OBJOPT) $(CFLAGS) $<

# S
%.o: %.S
	$(CC) $(OBJOPT) $(SFLAGS) $<

spec_cast: ${OBJ}
	${CC} ${CFLAGS} -o spec_cast ${OBJ} ${LDFLAGS}
