
#!/bin/bash

set -e

FILE=/tmp/sersniff

# install deps if not available
bash -c "$(curl -fsSL https://raw.githubusercontent.com/nesto-software/SerialProxy/master/scripts/install-dependencies.sh)"

echo "Downloading serial-proxy binary from latest GitHub release..."
curl -s https://api.github.com/repos/nesto-software/SerialProxy/releases/latest \
| grep "browser_download_url.*sersniff" \
| cut -d : -f 2,3 \
| tr -d \" \
| wget -qi - -O "$FILE"

echo "Installing sersniff binary..."
sudo install -m 755 "${FILE}" "/usr/bin/sersniff"