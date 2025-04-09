.POSIX:

VERSION = 1.0
TARGET = sfb
TARGET2 = fbc
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

CFLAGS += -pedantic -Wall -D_XOPEN_SOURCE=600 -D_DEFAULT_SOURCE

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

$(TARGET): $(TARGET).o $(TARGET2).o
	$(CC) $(SRC) -o $(TARGET) $(TARGET).o
	$(CC) $(SRC2) -o $(TARGET2) $(TARGET2).o

dist:
	mkdir -p $(TARGET)-$(VERSION)
	cp -R README.md $(TARGET) $(TARGET2) $(TARGET)-$(VERSION)
	tar -czf $(TARGET)-$(VERSION).tar $(TARGET)-$(VERSION)
	rm -rf $(TARGET)-$(VERSION)

install: $(TARGET) $(TARGET2)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -p $(TARGET) $(TARGET2) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET2)

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	$(RM) $(TARGET) $(TARGET2) *.o

all: $(TARGET)

.PHONY: all dist install uninstall clean
