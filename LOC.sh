#!/bin/sh
SYSROOT=root/orbit-kernel
echo $(( $(cat -s $SYSROOT/*.c \
       $SYSROOT/util/*.c \
       $SYSROOT/*.h \
       $SYSROOT/16bit/*.asm \
       $SYSROOT/32bit/*.c \
       $SYSROOT/32bit/include/*.h | wc -l) ))
