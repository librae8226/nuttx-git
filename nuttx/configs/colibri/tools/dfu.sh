#!/bin/bash

dfu-util -a1 -d 0483:df11 --alt 0 --dfuse-address 0x08000000 -D nuttx.bin -R
