LDFLAGS += -L.
CFLAGS   = -Wall -Wextra
LIBOBJS  = client.o comm.o encdec.o fid.o server.o
OBJS     = pd9p.o
MAKEDEPS = Makefile pd9p.h

.PHONY: all clean rebuild
all: libpd9p.a pd9p

pd9p: $(OBJS) libpd9p.a
	cc $(CFLAGS) $(LDFLAGS) -o pd9p $(OBJS) -lpd9p

libpd9p.a: $(LIBOBJS)
	rm -f libpd9p.a
	ar q libpd9p.a $(LIBOBJS)

%.o: %.c $(MAKEDEPS)
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f libpd9p.a $(LIBOBJS) $(OBJS)

rebuild: clean all
