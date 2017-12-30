timrec - Timed Media Recording Service
======================================
Copyright 2015-2017 Jiri Svoboda

Permission is hereby granted, free of charge, to any person obtaining 
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

Introduction
------------
timrec is a simple Linux service that allows for schedule-based recording
of media. It consists of a scheduler and recording backends. The 'wget'
backend can record web streams (e.g. internet radio) using the wget
command and the 'dvb' backend can record DVB broadcasts using dvb-tools
(dvbv5-zap).

timrec runs as a system service and is configured by CSV-like configuration
files. Systemd is required for service installation.

Compiling
---------

You need a working GNU toolchain (GCC, Binutils, Make) (Linux or similar OS).

To build the software simply type:

    $ make

To install the service and add it to the default runlevel run as root:

    # make install

To remove the service:

    # make uninstall

Installation directories
------------------------

Configuration
	/opt/timrec/etc/sched.txt
	/opt/timrec/etc/source.txt

Recorded files and recording logs are placed in
	/var/opt/timrec/

Service logs are in
	/var/log/timrec.log
	/var/log/timrec.err

Configuration
-------------

timrec is confured by two files. Example configuration files sched.txt
and source.txt are provided. These are then installed to /opt/timrec/etc/*.

All config files consist of series of records (rows), each row
consists of a list of key=value pairs separated by semicolons.
Rows starting with '#' are comments. Whitespace can be inserted in
arbitrary places.

If a value needs to contain spaces, it must be quoted. e.g. "CT 2"
if a quoted value needs to contain '"' or '\' characters, they need
to be escaped with a backslash: "\"" or "\\"

If the configuration is changed, the service must be restarted to
read the new configuration.

sched.txt

  Field name  | Field description
  ----------  | -----------------
  date or dow | If date is provided, recording will occur on this particular
              | date. Must be in the format YYYY-MM-DD. If dow is provided,
              | recording will occur always on the same day of week. Valid
              | values are 'po', 'ut', 'st', 'ct', 'pa', 'so', 'ne'
              | (for Monday-Sunday)
  time        | Nominal time of day when the programming starts.
              | Must be 24hr time format 'HH:MM'
  duration    | Nominal recording duration (with buffer if not exactly known)
              | This can be expressed in a combination of days, hours,
              | minutes and seconds. E.g. 1d6h13m25s
  source      | Name of recording source (from source.txt) to use
  name        | Name of the output file. A %D pattern will be replaced
              | with the recording date in the YYYY-MM-DD format.

Note that recording will start 10 minutes ahead of nominal programming
start time and will finish 15 minutes after the nominal end of
the programming. These buffers cannot be configured at the moment.

Example:

    dow=so; time=13:04; duration=120m; source=cro2; name=hra-%D

A programming that occurs every saturday at 13:04, takes 120 minutes,
recorded from cro2 source and the file will be named e.g.
hra-2017-11-30 if it is recorded on December 30th 2017.

source.txt

  Configures recording sources. Each source has the following parameters:

  Field name | Field description
  ---------- | -----------------
  name       | Name of the source that is used in the shedule
  rclass     | Recording class (wget or dvb)
  param      | Parameter (resource that will be recorded)

wget recording
--------------
    param must be set to the URI that is to be downloaded. This should
    be a web stream (e.g. internet radio)

dvb recording
-------------
    param must be set to the station name to be recorded. This should
    match a name in your $HOME/.tzap/channels.conf file
    If you don't have this file, it needs to be generated using
    the dvbv5-scan tool.

Example:

    name=cro2; rclass=wget; param=http://amp1.cesnet.cz:8000/cro2-256.ogg
    name=ct2; rclass=dvb; param="CT 2"

The first source is named 'cro2', records using wget backend from the
specified URI. The second records the DVB station named "CT 2" using
dvb-tools.
