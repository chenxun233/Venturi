#!/usr/bin/env bash
echo 1 | sudo tee /sys/bus/pci/devices/0000:06:00.0/remove
echo 1 | sudo tee /sys/bus/pci/rescan