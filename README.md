# About
This is an example complete and building project using the
[pico-sdk](https://github.com/raspberrypi/pico-sdk) to build a simple C/C++
application to run on the Seeed Xia RP2040 board.

The application blinks the onboard LED and prints messages out via the VCOM USB port.

Assumptions:
1. You are using Ubuntu-22.04 or better.
2. You are using the Seeed Xiao RP2040 board.
3. You have [picotool](https://github.com/raspberrypi/picotool) installed.
   If you do not, then the script below will try to clone the repo, build it,
   and install it.

# Build Instructions
```shell
sudo apt-get install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
git submodule update --init
if ! command -v picotool; then
    sudo apt-get install libusb-1.0-0-dev
    export PICO_SDK_PATH=$(realpath pico-sdk)
    tmpdir=$(mktemp -d)
    pushd "$tmpdir"
    git clone https://github.com/raspberrypi/picotool.git --branch master
    cd picotool
    cmake -B build -G Ninja
    cmake --build build
    sudo cmake --build build --target install
    popd
fi

cmake -B build -G Ninja
cmake --build build
```

# Flashing Instructions

Now, to flash the binary, you must hold the `BOOT` button on your board and then
connect it to your computer. To confirm that your board is connected and is ready
to receive a new firmware image, try running `sudo picotool info -a` and see if something
turns up. If so, then you are good to proceed! If not... ask an adult for help.

Now, let's get on with the flashing!

```shell
sudo picotool load build/blink.uf2
sudo picotool reboot
```

# Obersving the Results

You can visually inspect wether or not the on-board LED is blinking. As for the messages, you need to install some sort of serial terminal and open the COM port, which should now have magically appeared on reboot of the board.
