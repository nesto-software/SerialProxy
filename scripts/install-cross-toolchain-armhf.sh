#!/bin/bash
set -e

export DOWNLOAD_LOCATION=/tmp/crosstool-ng-armhf.tar.xz

apt-get install -y curl wget xz-utils tar

sudo mkdir -p /opt/crosstool-ng/x-tools/

echo "Downloading crosstool-ng toolchain for armhf..."
curl https://api.github.com/repos/nesto-software/crosstool-NG/releases/latest \
| grep "crosstool-ng-armhf.tar.xz" \
| cut -d : -f 2,3 \
| tr -d \" \
| wget -i - -O "$DOWNLOAD_LOCATION"

echo "Extract archive using tar"
sudo tar -xfv "$DOWNLOAD_LOCATION" -C /opt/crosstool-ng/x-tools/