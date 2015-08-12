#! /bin/bash
> checkRTC.log
> checkRTC.err
./adjustRTC.py
sudo watch -p -n15 ./checkRTC.sh
