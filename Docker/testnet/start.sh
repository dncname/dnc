#!/bin/sh
cd /opt/dnc/
./bios_boot_dnc.py -a --root ./
tail -f ./nodes/biosbpa.log