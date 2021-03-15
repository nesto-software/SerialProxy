#!/bin/bash

# for hardware proxy
sudo ./sersniff -i /dev/ttyACM0 -s -z

# for software proxy
sudo ./sersniff -i /dev/ttyUSB0 -o /dev/ttyUSB1 -z -b 19200

# alternative:
# sudo socat -x /dev/ttyUSB0,raw,b19200 /dev/ttyUSB1,raw,b19200
# ssh -i "Downloads/default_debug (20).id_ecdsa" pi@169.254.100.1