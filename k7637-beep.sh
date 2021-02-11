#!/bin/sh
#./k7637-beep.sh [duration]
DURATION=${1:-200}

# `xset led` does not work for me at all,
# so we use sysfs instead.
# This way we can also avoid sending the request to all attached keyboard.
for led in /sys/class/leds/*\:\:kana; do
	if [ "`cat $led/device/name`" = "VEB Kombinat Robotron K7637" ]; then
		# NOTE: This will usually require root
		echo 1 >$led/brightness || break
		sleep `printf '%.3f' ${DURATION}e-3`
		echo 0 >$led/brightness
		exit $?
	fi
done

# Fall back to regular PC speaker beep
exec beep -l $DURATION
