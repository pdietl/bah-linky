# About
This is a project using the
[pico-sdk](https://github.com/raspberrypi/pico-sdk) to build a C/C++
application to run on the Seeed Xia RP2040 board that controls a 6-key custom keyboard.
After flashing, the board should present itself to the host PC as a regular keyboard.
Using Capslock on a regular keyboard will make the LED blink on the board.
Pressing the 6 keys on the board will make the host PC type the numbers 1-6.

Assumptions:
1. You are using Ubuntu-22.04 or better.
2. You are using the Seeed Xiao RP2040 board.
3. You have [picotool](https://github.com/raspberrypi/picotool) installed.
   If you do not, then the script below will try to clone the repo, build it,
   and install it.

# Build Instructions

## Initial Setup
```shell
sudo apt-get install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
git submodule update --init
cd pico-sdk
git submodule update --init lib/tinyusb
cd ..

if ! command -v picotool; then
    sudo apt-get install libusb-1.0-0-dev
    export PICO_SDK_PATH=$(realpath pico-sdk)
    tmpdir=$(mktemp -d)
    pushd "$tmpdir"
    git clone https://github.com/raspberrypi/picotool.git --branch master
    cd picotool
    cmake -B build -G Ninja
    cmake --build build -j
    sudo cmake --build build --target install
    popd
fi
```

## Building the Project
```shell
cmake -B build -G Ninja
cmake --build build -j
```

# Flashing Instructions

Now, to flash the binary, you must hold the `BOOT` button on your board and then
connect it to your computer. To confirm that your board is connected and is ready
to receive a new firmware image, try running `sudo picotool info -a` and see if something
turns up. If so, then you are good to proceed! If not... ask an adult for help.

Now, let's get on with the flashing!

```shell
sudo picotool load build/app.uf2
sudo picotool reboot
```
