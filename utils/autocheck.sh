#! /bin/bash

while [ 1 ]
do
  ./checkRTC.py >> autocheck.log 2>> autocheck.err
  sleep 1h
done

