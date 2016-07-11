#!/bin/bash

ISOSXLION=$(sw_vers -productVersion)
case $ISOSXLION in
10.6)
	KEXT="stlink_shield10_6.kext"
    ;;
10.7)
    KEXT="stlink_shield10_7.kext"
    ;;
10.8)
    KEXT="stlink_shield10_8.kext"
    ;;
10.9)
    KEXT="stlink_shield10_9.kext"
    ;;
10.10)
    KEXT="stlink_shield10_10.kext"
    ;;    
*)
    echo "OS X version not supported."
    exit 1
    ;;
esac
chown -R root:wheel $KEXT/
cp -R $KEXT /System/Library/Extensions/stlink_shield.kext
kextload -v /System/Library/Extensions/stlink_shield.kext
touch /System/Library/Extensions
