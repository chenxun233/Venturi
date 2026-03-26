#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
  sudo ./port_isolation.sh <netdev>

Example:
  sudo ./port_isolation.sh enp1s0f1

This script tries to isolate a NIC port for replay/testing by:
  - flushing IPv4/IPv6 addresses
  - bringing the link up
  - disabling ARP
  - disabling IPv6 on the interface
  - asking NetworkManager to stop managing the device
  - stopping common discovery daemons if present
EOF
}

if [[ $# -ne 1 ]]; then
    usage
    exit 1
fi

if [[ ${EUID} -ne 0 ]]; then
    echo "error: run as root, for example: sudo ./port_isolation.sh $1" >&2
    exit 1
fi

DEV="$1"

if ! ip link show dev "$DEV" >/dev/null 2>&1; then
    echo "error: device '$DEV' not found" >&2
    exit 1
fi

run_optional() {
    if "$@"; then
        return 0
    fi
    return 0
}

echo "Isolating port: $DEV"

ip addr flush dev "$DEV"
ip -6 addr flush dev "$DEV" || true
ip link set dev "$DEV" up
ip link set dev "$DEV" arp off

sysctl -w "net.ipv6.conf.${DEV}.disable_ipv6=1"

if command -v nmcli >/dev/null 2>&1; then
    run_optional nmcli device set "$DEV" managed no
fi

run_optional systemctl stop lldpd
run_optional systemctl stop avahi-daemon
run_optional systemctl stop systemd-networkd

echo "Done."
echo "Current link state:"
ip -brief link show dev "$DEV"
