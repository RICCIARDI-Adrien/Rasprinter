# Rasprinter

Print labels using a Dymo LabelWriter 450 and a Raspberry Pi.

## Setup

Install the following packages on Raspbian :
```
sudo apt install printer-driver-dymo cups libsdl2-dev libsdl2-ttf-dev
```

### Installation

Type `sudo make install` to build the software, install it to `/opt` and install an init script.

### Uninstallation

Type `sudo make uninstall` to remove all installed files.
