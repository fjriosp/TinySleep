#! /usr/bin/env python

import serial
from datetime import datetime
import sys

rtc_time  = None
real_time = None

ser = serial.Serial('/dev/ttyUSB0', 9600)
ser.flushInput()
ser.flushOutput()

ser.write('p')
ser.readline();

while(rtc_time == None):
  ser.write('p')
  rtc = ser.readline().rstrip()
  real_time = datetime.now();
  try:
    rtc_time = datetime.strptime(rtc,"%Y-%m-%d %H:%M:%S.%f")
  except ValueError:
    print >> sys.stderr, "Could not convert "+rtc+" as datetime."

print str(real_time) + ' Offset: ' + str((rtc_time-real_time).total_seconds())
