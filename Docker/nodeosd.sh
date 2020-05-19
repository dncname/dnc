#!/bin/sh
cd /opt/dnc/bin

if [ -f '/opt/dnc/config/config.ini' ]; then
    echo 
  else
    echo "need to config config.ini"
    cp -R /tmp/config/config.ini /opt/dnc/config/
    exit 1
fi

if [ -f '/opt/dnc/config/genesis.json' ]; then
    echo
  else
    cp -R /tmp/config/* /opt/dnc/config/
fi

CONFIG_DIR="--config-dir /opt/dnc/config/ -d /opt/dnc/data/"

exec /opt/dnc/bin/nodeos $CONFIG_DIR $@
