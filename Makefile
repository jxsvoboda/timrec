#
# Timrec - timed media recording service
#
# Copyright 2017 Jiri Svoboda
#
# Permission is hereby granted, free of charge, to any person obtaining 
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#


CC = gcc
CFLAGS = -Wall -Werror -I. -std=gnu99 -D_GNU_SOURCE
LIBS = -lrt

output = timrec
rcscript = timrec.init
rcname = timrec
prefix = /opt/timrec

sources = \
	adt/list.c \
	field.c \
	preset.c \
	rclass/dvb.c \
	rclass/wget.c \
	revent.c \
	rsession.c \
	sched.c \
	source.c \
	startspec.c \
	timrec.c

objects = $(sources:.c=.o)
headers = $(wildcard *.h */*.h)

bkqual = $$(date '+%Y-%m-%d')

all: $(output)

$(output): $(objects)
	$(CC) $(LIBS) -o $@ $^

$(objects): $(headers)

clean:
	rm -f $(output) $(objects)

install:
	mkdir -p /var/opt/timrec
	mkdir -p $(prefix)/bin
	mkdir -p $(prefix)/etc
	install -o root -g root -m 755 $(output) $(prefix)/bin/$(output)
	install -o root -g root -m 755 $(rcscript) $(prefix)/bin/$(rcscript)
	install -o root -g root -m 644 sched.txt $(prefix)/etc/sched.txt
	install -o root -g root -m 644 source.txt $(prefix)/etc/source.txt
	ln -s $(prefix)/bin/$(rcscript) /etc/init.d/$(rcname)
	update-rc.d $(rcname) defaults

uninstall:
	update-rc.d $(rcname) remove
	rm -f /etc/init.d/$(rcname)
	rm -f $(prefix)/bin/$(output)
	rm -f $(prefix)/bin/$(rcscript)
	rmdir $(prefix)/bin
	rmdir $(prefix)

backup: clean
	cd .. && tar czf timrec-$(bkqual).tar.gz trunk
	cd .. && rm -f timrec-latest.tar.gz && ln -s timrec-$(bkqual).tar.gz timrec-latest.tar.gz
