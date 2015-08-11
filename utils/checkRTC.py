#! /usr/bin/env python

import serial
import time
import sys

rtc_time  = None
real_time = None

ser = serial.Serial('/dev/ttyUSB0', 9600)
ser.flushInput()
ser.flushOutput()

while(rtc_time == None):
  rtc = ser.readline().rstrip()
  real_time = time.time();
  try:
    rtc_time = time.mktime(time.strptime(rtc,"%Y-%m-%d %H:%M:%S"))
  except ValueError:
    print >> sys.stderr, "Could not convert "+rtc+" as datetime."

print time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(real_time)) + ' Offset: ' + str(rtc_time-real_time)
