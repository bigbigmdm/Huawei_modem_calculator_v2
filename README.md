# Huawei_modem_calculator_v2

Is it [Huawei modem unlock code calculator](https://github.com/forth32/huaweicalc) from **forth32** with the USB serial interface.

![Screenshot](https://github.com/bigbigmdm/Huawei_modem_calculator_v2/blob/main/img/huawei_calc_2.png)

This program requires the installation of the ssl development library.
On Red Hat, Centos, Fedora, Redos you can install it by running the following command:

**`sudo dnf install openssl-devel`**

**`sudo dnf install qt5-qtserialport-devel cmake `**

**`sudo dnf install make automake gcc gcc-c++ kernel-devel`**

On Debian, Ubuntu, MX, Linux Mint you can install it by running the following command:

**`sudo apt-get install libssl-dev`**

**`sudo apt install libqt5serialport5-dev cmake build-essential`**

To compile and install the program, run the command `sudo ./install.sh`.
To uninstall the program, run the command `sudo ./uninstall.sh`.
