#! /usr/bin/env python

import serial
import time

rtc_time  = None
real_time = None

ser = serial.Serial('/dev/ttyUSB0', 9600)
ser.flushInput()
ser.flushOutput()
ser.write('p')

time.sleep(1)

ser.write('s'+time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()))

