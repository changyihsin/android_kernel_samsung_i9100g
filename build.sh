#!/bin/bash

# Environment setup
export ARCH=arm
export CROSS_COMPILE="/home/netchip/android/google/ics/prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/bin/arm-eabi-"

# Clean
make mrproper

# Defconfig
make t1_netchip_defconfig

# Modules
make -j4 

# Copy modules
find -name '*.ko' -exec cp -av {} usr/initramfs/lib/modules/ \;

# Kernel
make -j4 
