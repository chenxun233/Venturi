#!/usr/bin/env bash
set -euo pipefail

NS_NAME="${NS_NAME:-dummy-server}"
IFACE="${IFACE:-enp6s0f0}"
SERVER_IP_CIDR="${SERVER_IP_CIDR:-192.168.51.2/30}"
SERVER_IP="${SERVER_IP:-192.168.51.2}"
SERVER_PORT="${SERVER_PORT:-9000}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DUMMY_SERVER_BIN="${DUMMY_SERVER_BIN:-$REPO_ROOT/cpp_src/build/dummy_server}"

need_root() {
  if [[ "${EUID}" -ne 0 ]]; then
    echo "This script must run as root. Example: sudo $0 $*" >&2
    exit 1
  fi
}

ns_exists() {
  ip netns list | awk '{print $1}' | grep -qx "$NS_NAME"
}

setup_ns() {
  need_root "$@"

  if ! ns_exists; then
    ip netns add "$NS_NAME"
  fi

  ip netns exec "$NS_NAME" ip link set lo up

  if ip link show "$IFACE" >/dev/null 2>&1; then
    ip link set "$IFACE" down
    ip link set "$IFACE" netns "$NS_NAME"
  fi

  ip -n "$NS_NAME" addr flush dev "$IFACE" || true
  ip -n "$NS_NAME" addr add "$SERVER_IP_CIDR" dev "$IFACE"
  ip -n "$NS_NAME" link set "$IFACE" up

  echo "Namespace $NS_NAME ready"
  ip -n "$NS_NAME" -br addr show dev "$IFACE"
  echo
  echo "Route check from default namespace:"
  ip route get "$SERVER_IP"
}

start_server() {
  need_root "$@"

  if ! ns_exists; then
    echo "Namespace $NS_NAME does not exist. Run: sudo $0 setup" >&2
    exit 1
  fi

  if [[ ! -x "$DUMMY_SERVER_BIN" ]]; then
    echo "dummy_server binary not found or not executable: $DUMMY_SERVER_BIN" >&2
    exit 1
  fi

  exec ip netns exec "$NS_NAME" "$DUMMY_SERVER_BIN" --listen-ip "$SERVER_IP" --port "$SERVER_PORT"
}

status_ns() {
  if ns_exists; then
    echo "Namespace $NS_NAME exists"
    ip -n "$NS_NAME" -br addr show || true
    echo
    ip route get "$SERVER_IP" || true
    echo
    ip netns exec "$NS_NAME" ss -ltnp || true
  else
    echo "Namespace $NS_NAME does not exist"
  fi
}

cleanup_ns() {
  need_root "$@"

  if ns_exists; then
    if ip -n "$NS_NAME" link show "$IFACE" >/dev/null 2>&1; then
      ip -n "$NS_NAME" link set "$IFACE" down
      ip -n "$NS_NAME" link set "$IFACE" netns 1
      ip addr flush dev "$IFACE" || true
      ip addr add "$SERVER_IP_CIDR" dev "$IFACE"
      ip link set "$IFACE" up
    fi
    ip netns del "$NS_NAME"
  fi

  echo "Namespace $NS_NAME removed"
  ip -br addr show dev "$IFACE" || true
  echo
  ip route get "$SERVER_IP" || true
}

case "${1:-}" in
  setup)
    setup_ns "$@"
    ;;
  start)
    start_server "$@"
    ;;
  status)
    status_ns
    ;;
  cleanup)
    cleanup_ns "$@"
    ;;
  *)
    cat <<'EOF'
Usage:
  sudo ./scripts/dummy_server_netns.sh setup    # create namespace, move enp6s0f0 into it, assign 192.168.51.2/30
  sudo ./scripts/dummy_server_netns.sh start    # run dummy_server inside the namespace (foreground)
  ./scripts/dummy_server_netns.sh status        # inspect namespace, routing, and listeners
  sudo ./scripts/dummy_server_netns.sh cleanup  # move enp6s0f0 back to the default namespace
EOF
    exit 1
    ;;
esac
