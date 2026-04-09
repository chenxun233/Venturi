#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
  sudo ./port_isolation.sh <netdev> [<netdev> ...]

Example:
  sudo ./port_isolation.sh enp1s0f1
  sudo ./port_isolation.sh enp5s0f0 enp5s0f1 enp6s0f0 enp6s0f1

This script tries to isolate a NIC port for replay/testing by:
  - flushing IPv4/IPv6 addresses
  - bringing the link up
  - disabling ARP
  - disabling IPv6 on the interface
  - asking NetworkManager to stop managing the device
  - stopping common discovery daemons if present
EOF
}

run_optional() {
    if "$@"; then
        return 0
    fi
    return 0
}

isolate_port() {
    local dev="$1"

    if ! ip link show dev "$dev" >/dev/null 2>&1; then
        echo "error: device '$dev' not found" >&2
        return 1
    fi

    echo "Isolating port: $dev"

    ip addr flush dev "$dev" || return $?
    ip -6 addr flush dev "$dev" || true
    ip link set dev "$dev" up || return $?
    ip link set dev "$dev" arp off || return $?

    sysctl -w "net.ipv6.conf.${dev}.disable_ipv6=1" || return $?

    if command -v nmcli >/dev/null 2>&1; then
        run_optional nmcli device set "$dev" managed no
    fi

    run_optional systemctl stop lldpd
    run_optional systemctl stop avahi-daemon
    run_optional systemctl stop systemd-networkd

    echo "Current link state:"
    ip -brief link show dev "$dev" || return $?
}

isolate_ports() {
    local failed_ports=()
    local dev

    for dev in "$@"; do
        if ! isolate_port "$dev"; then
            echo "failed to isolate port: $dev" >&2
            failed_ports+=("$dev")
        fi
    done

    if [[ ${#failed_ports[@]} -gt 0 ]]; then
        echo "Failed ports: ${failed_ports[*]}" >&2
        return 1
    fi

    echo "Done."
}

main() {
    if [[ $# -lt 1 ]]; then
        usage
        return 1
    fi

    if [[ ${EUID} -ne 0 ]]; then
        echo "error: run as root, for example: sudo ./port_isolation.sh $1" >&2
        return 1
    fi

    isolate_ports "$@"
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    main "$@"
fi
