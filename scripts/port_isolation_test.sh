#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

mkdir -p "$tmpdir/bin"
log_file="$tmpdir/command.log"

cat >"$tmpdir/bin/ip" <<EOF
#!/usr/bin/env bash
set -euo pipefail
printf 'ip %s\n' "\$*" >>"$log_file"

if [[ "\$1 \$2 \$3" == "link show dev" ]]; then
    case "\$4" in
        good0|good1|bad0)
            exit 0
            ;;
        *)
            exit 1
            ;;
    esac
fi

if [[ "\$1 \$2 \$3" == "-brief link show" ]]; then
    printf '%s UP\n' "\$5"
    exit 0
fi

if [[ "\$1 \$2" == "addr flush" && "\$4" == "bad0" ]]; then
    exit 42
fi

if [[ "\$1 \$2 \$3" == "-6 addr flush" && "\$4" == "bad0" ]]; then
    exit 0
fi

exit 0
EOF
chmod +x "$tmpdir/bin/ip"

cat >"$tmpdir/bin/sysctl" <<EOF
#!/usr/bin/env bash
set -euo pipefail
printf 'sysctl %s\n' "\$*" >>"$log_file"
exit 0
EOF
chmod +x "$tmpdir/bin/sysctl"

cat >"$tmpdir/bin/nmcli" <<EOF
#!/usr/bin/env bash
set -euo pipefail
printf 'nmcli %s\n' "\$*" >>"$log_file"
exit 0
EOF
chmod +x "$tmpdir/bin/nmcli"

cat >"$tmpdir/bin/systemctl" <<EOF
#!/usr/bin/env bash
set -euo pipefail
printf 'systemctl %s\n' "\$*" >>"$log_file"
exit 0
EOF
chmod +x "$tmpdir/bin/systemctl"

if output="$(
    PATH="$tmpdir/bin:$PATH" \
    bash -lc '
        set -euo pipefail
        source "'"$SCRIPT_DIR"'/port_isolation.sh"
        isolate_ports good0 missing0 bad0 good1
    ' 2>&1
)"; then
    status=0
else
    status=$?
fi

if [[ $status -ne 1 ]]; then
    echo "expected isolate_ports to exit with status 1, got $status" >&2
    echo "$output" >&2
    exit 1
fi

grep -F "Isolating port: good0" <<<"$output" >/dev/null
grep -F "error: device 'missing0' not found" <<<"$output" >/dev/null
grep -F "failed to isolate port: bad0" <<<"$output" >/dev/null
grep -F "Isolating port: good1" <<<"$output" >/dev/null
grep -F "Failed ports: missing0 bad0" <<<"$output" >/dev/null

grep -F "ip addr flush dev good0" "$log_file" >/dev/null
grep -F "ip addr flush dev good1" "$log_file" >/dev/null

echo "port_isolation_test: PASS"
