#!/bin/bash
if [ "$EUID" -ne 0 ]
  then echo "Please run as root! (sudo ./uninstall.sh)"
  exit
fi
rm -rf /usr/bin/Huawei_modem_calculator
rm -rf /usr/share/applications/Huawei_modem_calculator.desktop
rm -rf /usr/share/pixmaps/huawei_unlocker64.png
rm -rf /etc/udev/rules.d//50-myusb.rules
