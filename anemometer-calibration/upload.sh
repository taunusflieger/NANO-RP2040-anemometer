#!/bin/sh
arduino-cli upload -p /dev/ttyACM0 --fqbn "STMicroelectronics:stm32:GenF4:pnum=FEATHER_F405,upload_method=dfuMethod,xserial=generic,usb=CDCgen,xusb=FS,opt=osstd,dbg=none,rtlib=nano"
