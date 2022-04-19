#!/bin/sh
arduino-cli compile -v --fqbn "STMicroelectronics:stm32:GenF4:pnum=FEATHER_F405,upload_method=dfuMethod,xserial=generic,usb=CDCgen,xusb=FS,opt=osstd,dbg=none,rtlib=nano"
