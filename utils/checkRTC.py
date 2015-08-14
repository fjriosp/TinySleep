#! /usr/bin/env python

import argparse
import serial
import time
from datetime import datetime
import sys

dt2000 = datetime(2000,1,1,0,0,0,0)
t2000  = time.mktime(dt2000.timetuple())

def setRTCTime(current=True):
  ser = serial.Serial('/dev/ttyUSB0', 9600)
  ser.flushInput()
  ser.flushOutput()

  ser.write(' ')
  time.sleep(1)
  ser.readline();

  if(current):
    ser.write('s'+time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()))
  else:
    ser.write('s2000-01-01 00:00:00')

  ser.close()

def checkTime():
  rtc_time  = None
  real_time = None

  ser = serial.Serial('/dev/ttyUSB0', 9600)
  ser.flushInput()
  ser.flushOutput()

  ser.write(' ')
  ser.readline();

  while(rtc_time == None):
    ser.write('p')
    rtc = ser.readline().rstrip()
    real_time = time.time();
    try:
      t = datetime.strptime(rtc,"%Y-%m-%d %H:%M:%S.%f")
      rtc_time = time.mktime(t.timetuple()) + t.microsecond / 1E6
    except ValueError:
      print >> sys.stderr, "Could not convert "+rtc+" as datetime."
  
  ser.close();
  return (real_time,rtc_time-real_time)

def formatTime(t):
  return datetime.fromtimestamp(t).strftime("%Y-%m-%d %H:%M:%S.%f")

def printAdjust(t,o,ft,fo):
  d  = (o-fo)/(t-ft)  # d/sec
  d *= 60             # d/min
  tt = d/(0.250/256)  # t/min
  am = round(tt)
  tt = (tt-am)*60     # t/hour
  ah = round(tt)
  tt = (tt-ah)*24     # t/day
  ad = round(tt)
  tt = (tt-ad)        # e/day

  print "%s T:%s O:% 12.6f A: %+04d %+04d %+04d" % (formatTime(t),formatTime(t2000+t-ft),o-fo,am,ah,ad)

########
# main #
########
parser = argparse.ArgumentParser(description='Checks the RTC precision.')
parser.add_argument('-r','--reset', help='Clears the RTC clock',action="store_true")
parser.add_argument('-s','--sync',  help='Synchronizes the RTC Clock with the system clock',action="store_true")
parser.add_argument('-c','--check', help='Checks the RTC precision',action="store_true")
parser.add_argument('-w','--wait',  help='Interval to check [default: 30 min]',default=30*60, type=int)
args = parser.parse_args()

if(args.reset):
  setRTCTime(False)
  print "RTC reset"

if(args.sync):
  setRTCTime()
  print "RTC sync"

if(args.check):
  print "Starting RTC check"
  (ft,fo) = checkTime()
  print "%s O:% 12.6f" % (formatTime(ft),fo)

  while True:
    sys.stdout.flush()
    time.sleep(args.wait)
    (t,o) = checkTime()
    printAdjust(t,o,ft,fo)

