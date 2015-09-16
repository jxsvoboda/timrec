#
# Timrec - timed media recording service
# (c) 2015 Jiri Svoboda
#

CC = gcc
CFLAGS = -Wall -Werror -I. -std=gnu99 -D_GNU_SOURCE
LIBS =

output = timrec
rcscript = timrec.init
rcname = timrec
prefix = /opt/timrec

sources = \
	adt/list.c \
	preset.c \
	rclass/wget.c \
	revent.c \
	rsession.c \
	sched.c \
	startspec.c \
	timrec.c

objects = $(sources:.c=.o)
headers = $(wildcard *.h */*.h)

all: $(output)

$(output): $(objects)
	$(CC) $(LIBS) -o $@ $^

$(objects): $(headers)

clean:
	rm -f $(output) $(objects)

install:
	mkdir -p /var/opt/timrec
	mkdir -p $(prefix)/bin
	install -o root -g root -m 755 $(output) $(prefix)/bin/$(output)
	install -o root -g root -m 755 $(rcscript) $(prefix)/bin/$(rcscript)
	ln -s $(prefix)/bin/$(rcscript) /etc/init.d/$(rcname)
	update-rc.d $(rcname) defaults

uninstall:
	update-rc.d $(rcname) remove
	rm -f /etc/init.d/$(rcname)
	rm -f $(prefix)/bin/$(output)
	rm -f $(prefix)/bin/$(rcscript)
	rmdir $(prefix)/bin
	rmdir $(prefix)
