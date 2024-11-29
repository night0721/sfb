.POSIX:
.SUFFIXES:

VERSION = 1.0
TARGET = sfb
TARGET2 = fbc
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

CFLAGS = -Os -march=native -mtune=native -pipe -s -std=c99 -pedantic -Wall -D_XOPEN_SOURCE=600

SRC = $(TARGET).c
SRC2 = $(TARGET2).c

$(TARGET): $(SRC) $(SRC2)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)
	$(CC) $(SRC2) -o $(TARGET2) $(CFLAGS)

dist:
	mkdir -p $(TARGET)-$(VERSION)
	cp -R README.md $(TARGET) $(TARGET2) $(TARGET)-$(VERSION)
	tar -cf $(TARGET)-$(VERSION).tar $(TARGET)-$(VERSION)
	gzip $(TARGET)-$(VERSION).tar
	rm -rf $(TARGET)-$(VERSION)

install: $(TARGET) $(TARGET2)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -p $(TARGET) $(TARGET2) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET2)

uninstall:
	rm $(DESTDIR)$(BINDIR)/$(TARGET)
	rm $(DESTDIR)$(BINDIR)/$(TARGET2)

clean:
	rm $(TARGET)
	rm $(TARGET2)

all: $(TARGET)

.PHONY: all dist install uninstall clean
