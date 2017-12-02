CFLAGS=-std=c99 -D_BSD_SOURCE

PROG=newfs_msdos

$(PROG): mkfs_msdos.o newfs_msdos.o
	${CC} -o $(PROG) mkfs_msdos.o newfs_msdos.o

clean:
	rm -f *.o $(PROG)

install: $(PROG)
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install -m755 $(PROG) $(DESTDIR)/$(PREFIX)/bin/
