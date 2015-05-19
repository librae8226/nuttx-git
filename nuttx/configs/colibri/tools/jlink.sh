#!/bin/bash

if [ is$1 = 'isdfu' ]; then
	JLinkExe -CommanderScript configs/colibri/tools/jlink.dfu.cmd
else
	JLinkExe -CommanderScript configs/colibri/tools/jlink.cmd
fi
