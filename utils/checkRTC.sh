#! /bin/bash

ntpdate ntp.ubuntu.com >> checkRTC.log 2>> checkRTC.err
./checkRTC.py >> checkRTC.log 2>> checkRTC.err
./checkRTC.py >> checkRTC.log 2>> checkRTC.err
./checkRTC.py >> checkRTC.log 2>> checkRTC.err

