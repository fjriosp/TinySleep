#! /usr/bin/env python

import os
import argparse
import serial
import time
from datetime import datetime
import sys

dt2000 = datetime(2000,1,1,0,0,0,0)
t2000  = time.mktime(dt2000.timetuple())

def wakeUp():
  ser = serial.Serial('/dev/ttyUSB0', 9600)

  ser.write('\r')
  ser.flushOutput()
  time.sleep(1)
  ser.write('\r')
  ser.flushOutput()
  time.sleep(1)
  ser.flushInput()

  return ser

def setRTCTime(current=True):
  ser = wakeUp()

  if(current):
    ser.write('s'+datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]+'\r')
  else:
    ser.write('s2000-01-01 00:00:00.000\r')

  ser.close()

def checkTime():
  rtc_time  = None
  real_time = None

  ser = wakeUp()

  while(rtc_time == None):
    ser.write('p\r')
    rtc = ser.readline().rstrip()
    real_time = time.time();
    try:
      t = datetime.strptime(rtc,"%Y-%m-%d %H:%M:%S.%f")
      rtc_time = time.mktime(t.timetuple()) + t.microsecond / 1E6
    except ValueError:
      print >> sys.stderr, "Could not convert "+rtc+" as datetime."
  
  ser.close();

  res = dict()
  res['time']   = real_time
  res['rtc']    = rtc_time
  res['rtc_str']= rtc
  res['offset'] = rtc_time-real_time
  return res

def printRTCTime():
  rtc = None

  ser = wakeUp()
  ser.write('p\r')
  print ser.readline().rstrip()
  ser.close();

def printRTCAdjust():
  rtc = None

  ser = wakeUp()
  ser.write('a\r')
  print ser.readline().rstrip()
  ser.close();

def formatTime(t):
  return datetime.fromtimestamp(t).strftime("%Y-%m-%d %H:%M:%S.%f")

def printAdjust(r,s):
  fo = s['offset']
  ft = s['time']
  o  = r['offset']
  t  = r['time']
  rtc= r['rtc']

  d  = (fo-o)/(t-ft)  # d/sec
  d *= 60             # d/min
  tt = d/(0.250/256)  # t/min
  am = round(tt)
  tt = (tt-am)*60     # t/hour
  ah = round(tt)
  tt = (tt-ah)*24     # t/day
  ad = round(tt)
  tt = (tt-ad)        # e/day

  print "%s T:%s R:%s O:% 12.6f A: %+04d %+04d %+04d" % (formatTime(t),formatTime(rtc),formatTime(t2000+t-ft),o-fo,am,ah,ad)

def loadSession(fn):
  f = open(fn, 'r')
  s = eval(f.read())
  f.close()
  return s

def saveSession(fn, s):
  f = open(fn, 'w')
  f.write(str(s))
  f.close()

########
# main #
########
parser = argparse.ArgumentParser(description='Checks the RTC precision.')
parser.add_argument('-a','--printAdj', help='Show the RTC adjust',action="store_true")
parser.add_argument('-p','--printRTC', help='Show the RTC clock',action="store_true")
parser.add_argument('-r','--reset'   , help='Clears the RTC clock',action="store_true")
parser.add_argument('-S','--session' , help='Session file',default=None)
parser.add_argument('-s','--sync'    , help='Synchronizes the RTC Clock with the system clock',action="store_true")
parser.add_argument('-c','--check'   , help='Checks the RTC precision',action="store_true")
parser.add_argument('-w','--wait'    , help='Interval to check [default: 30 min]',default=30*60, type=int)
args = parser.parse_args()

if(args.reset):
  print "RTC reset"
  setRTCTime(False)
  sys.stdout.flush()

if(args.sync):
  print "RTC sync"
  setRTCTime()
  sys.stdout.flush()

if(args.printRTC):
  printRTCTime()
  sys.stdout.flush()

if(args.printAdj):
  printRTCAdjust()
  sys.stdout.flush()

session = None

if(args.session != None):
  if(args.reset or args.sync or not os.path.isfile(args.session)):
    print "Saving RTC session"
    session = checkTime()
    saveSession(args.session,session)
  else:
    print "Loading RTC session"
    session = loadSession(args.session)

if(session == None):
  print "No session found"
  session = checkTime()

if(args.check):
  print "Starting RTC check"
  ft = session['time']
  fo = session['offset']
  print "%s O:% 12.6f" % (formatTime(ft),fo)

  while True:
    tmp = checkTime()
    printAdjust(tmp,session)
    sys.stdout.flush()
    time.sleep(args.wait)

