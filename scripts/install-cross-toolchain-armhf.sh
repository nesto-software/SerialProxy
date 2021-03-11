#!/bin/bash
set -e

FILE=/tmp/crosstool-ng-armhf.tar.xz

apt-get install -y curl wget xz-utils

mkdir -p /opt/crosstool-ng/x-tools/

echo "Downloading crosstool-ng toolchain for armhf..."
curl -s https://api.github.com/repos/nesto-software/crosstool-NG/releases/latest \
| grep "crosstool-ng-armhf.tar.xz" \
| cut -d : -f 2,3 \
| tr -d \" \
| wget -qi - -O "$FILE"

sudo tar xfv /tmp/crosstool-ng-armhf.tar.xz -C /opt/crosstool-ng/x-tools/