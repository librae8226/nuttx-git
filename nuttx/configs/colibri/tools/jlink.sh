#!/bin/bash

if [ is$1 = 'isrecovery' ]; then
	JLinkExe -CommanderScript configs/colibri/tools/jlink.recovery.cmd
else
	JLinkExe -CommanderScript configs/colibri/tools/jlink.cmd
fi
