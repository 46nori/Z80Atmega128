#!/bin/bash
#
# Create CP/M disk image
#
if [ $# -l 3 ]; then
  echo "Usage: mkimg.sh <fmt> <img> [files ...]" 1>&2
  echo "       <fmt>: format defined in /etc/cpmtools/diskdefs" 1>&2
  echo "       <img>: image file name" 1>&2
  echo "       [files ...]: CP/M files (optional)" 1>&2
  exit 1
fi

USR="0:"
FMT=$1
IMG=$2
FILES="${@:3:($#-2)}"

mkfs.cpm -f ${FMT} ${IMG}
dd if=/dev/zero of=${IMG} bs=8192 count=1021 oflag=append conv=notrunc
if [ -n "${FILES}" ]; then
    cpmcp -f ${FMT} ${IMG} ${FILES} ${USR}
fi
cpmls -f ${FMT} ${IMG}
