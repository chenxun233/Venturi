#!/usr/bin/env bash
set -euo pipefail

usage() {
	echo "Usage: $0 [options] [BDF|vendor:device]"
	echo "  BDF example: 0000:05:00.0 or 05:00.0"
	echo "  ID  example: 10ee:903f (default when omitted: vendor 10ee)"
	echo
	echo "Options:"
	echo "  --allow-first-match   If multiple devices match, use the first one."
	echo "  --no-driver-cycle     Skip driver unbind/rebind steps."
	echo "  Note: if BDF is already missing, parent bridge reset+rescan is used."
}

to_full_bdf() {
	local bdf="$1"
	if [[ "${bdf}" =~ ^[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\.[0-7]$ ]]; then
		printf "%s\n" "${bdf,,}"
	elif [[ "${bdf}" =~ ^[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\.[0-7]$ ]]; then
		printf "0000:%s\n" "${bdf,,}"
	else
		return 1
	fi
}

find_by_id() {
	local pattern="$1"
	lspci -Dnn | awk -v p="${pattern}" '
		BEGIN { IGNORECASE=1 }
		index($0, p) > 0 { print $1 }
	'
}

find_by_bus() {
	local bus="$1"
	lspci -Dnn | awk -v b="${bus}" '
		$1 ~ ("^0000:" b ":") { print $1 }
	'
}

find_parent_by_bus() {
	local bus="$1"
	local sec_file sec_val sec_val_lc sec_num
	local want_num=$((16#${bus}))

	for sec_file in /sys/bus/pci/devices/*/secondary_bus_number; do
		[[ -e "${sec_file}" ]] || continue
		sec_val="$(<"${sec_file}")"
		sec_val_lc="${sec_val,,}"
		if [[ "${sec_val_lc}" =~ ^0x[0-9a-f]+$ ]]; then
			sec_num=$((16#${sec_val_lc#0x}))
			if (( sec_num == want_num )); then
				basename "${sec_file%/secondary_bus_number}"
				return 0
			fi
		fi
	done

	return 1
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
	usage
	exit 0
fi

if (( EUID != 0 )); then
	exec sudo "$0" "$@"
fi

if ! command -v lspci >/dev/null 2>&1; then
	echo "Error: lspci not found (install pciutils)." >&2
	exit 1
fi

target=""
endpoint=""
allow_first_match=0
driver_cycle=1

while (( $# > 0 )); do
	case "$1" in
		-h|--help)
			usage
			exit 0
			;;
		--allow-first-match)
			allow_first_match=1
			;;
		--no-driver-cycle)
			driver_cycle=0
			;;
		--)
			shift
			break
			;;
		-*)
			echo "Error: unknown option '$1'" >&2
			usage
			exit 1
			;;
		*)
			if [[ -n "${target:-}" ]]; then
				echo "Error: multiple targets provided ('$target' and '$1')." >&2
				usage
				exit 1
			fi
			target="$1"
			;;
	esac
	shift
done

if (( $# > 0 )); then
	echo "Error: unexpected extra arguments: $*" >&2
	usage
	exit 1
fi

target="${target:-10ee:}"
missing_endpoint=0
parent=""
bus_hex=""
id_pat=""

if endpoint="$(to_full_bdf "${target}" 2>/dev/null)"; then
	bus_hex="${endpoint:5:2}"
	if [[ ! -d "/sys/bus/pci/devices/${endpoint}" ]]; then
		if parent="$(find_parent_by_bus "${bus_hex}")"; then
			echo "Warning: PCI device ${endpoint} not present in sysfs; switching to parent-bridge recovery on bus ${bus_hex}." >&2
			missing_endpoint=1
		else
			echo "Error: PCI device ${endpoint} not found in sysfs and no parent bridge found for bus ${bus_hex}." >&2
			exit 1
		fi
	fi
else
	mapfile -t matches < <(find_by_id "${target}")
	if (( ${#matches[@]} == 0 )); then
		echo "Error: no PCI device matching '${target}' found." >&2
		exit 1
	fi
	if (( ${#matches[@]} > 1 )); then
		if (( allow_first_match == 0 )); then
			echo "Error: multiple devices matched '${target}'." >&2
			printf '  %s\n' "${matches[@]}" >&2
			echo "Pass an explicit BDF, or use --allow-first-match to override." >&2
			exit 1
		fi
		echo "Warning: multiple devices matched '${target}', using first: ${matches[0]}" >&2
		printf '  %s\n' "${matches[@]}" >&2
	fi
	endpoint="${matches[0]}"
	bus_hex="${endpoint:5:2}"
fi

if (( missing_endpoint == 0 )); then
	parent="$(basename "$(readlink -f "/sys/bus/pci/devices/${endpoint}/..")")"
	if [[ -z "${parent}" || "${parent}" == "${endpoint}" ]]; then
		echo "Error: failed to determine upstream bridge for ${endpoint}." >&2
		exit 1
	fi
fi

vendor=""
device=""
if (( missing_endpoint == 0 )); then
	vendor="$(<"/sys/bus/pci/devices/${endpoint}/vendor")"
	device="$(<"/sys/bus/pci/devices/${endpoint}/device")"
	id_pat="[${vendor#0x}:${device#0x}]"
fi

if (( missing_endpoint == 0 )); then
	echo "Target endpoint : ${endpoint} (${vendor#0x}:${device#0x})"
else
	echo "Target endpoint : ${endpoint} (missing)"
	echo "Target bus      : ${bus_hex}"
fi
echo "Upstream bridge : ${parent}"
if (( driver_cycle == 0 )); then
	echo "Driver cycle    : disabled (--no-driver-cycle)"
fi

saved_driver=""
if (( missing_endpoint == 0 )) && (( driver_cycle == 1 )) && [[ -L "/sys/bus/pci/devices/${endpoint}/driver" ]]; then
	saved_driver="$(basename "$(readlink -f "/sys/bus/pci/devices/${endpoint}/driver")")"
	if [[ -n "${saved_driver}" ]]; then
		echo "Unbinding driver : ${saved_driver}"
		echo "${endpoint}" > "/sys/bus/pci/devices/${endpoint}/driver/unbind"
	fi
fi

if (( missing_endpoint == 0 )) && [[ -e "/sys/bus/pci/devices/${endpoint}/remove" ]]; then
	echo "Removing endpoint from PCI bus..."
	echo 1 > "/sys/bus/pci/devices/${endpoint}/remove"
fi

did_parent_reset=0
if [[ -e "/sys/bus/pci/devices/${parent}/reset" ]]; then
	echo "Resetting upstream bridge via sysfs reset..."
	echo 1 > "/sys/bus/pci/devices/${parent}/reset"
	did_parent_reset=1
fi

if (( did_parent_reset == 0 )); then
	if command -v setpci >/dev/null 2>&1; then
		echo "Applying PCIe hot reset via BRIDGE_CONTROL on ${parent}..."
		orig="$(setpci -s "${parent}" BRIDGE_CONTROL)"
		if [[ -z "${orig}" ]]; then
			echo "Error: failed to read BRIDGE_CONTROL from ${parent}." >&2
			exit 1
		fi
		orig="${orig,,}"
		new="$(printf "%04x" $(( 0x${orig} | 0x0040 )))"
		setpci -s "${parent}" BRIDGE_CONTROL="${new}"
		sleep 0.2
		setpci -s "${parent}" BRIDGE_CONTROL="${orig}"
	else
		echo "Warning: setpci not available and no sysfs reset on upstream bridge." >&2
	fi
fi

echo "Rescanning PCI bus..."
echo 1 > /sys/bus/pci/rescan
sleep 1

rebind_endpoint=""
if (( missing_endpoint == 1 )); then
	mapfile -t found_after < <(find_by_bus "${bus_hex}")
	if (( ${#found_after[@]} == 0 )); then
		echo "No device found on bus ${bus_hex} after parent reset+rescan."
		echo "If FPGA is still missing, verify PCIe init in bitstream and try a cold power cycle."
		exit 2
	fi
	echo "Found device(s) on bus ${bus_hex} after rescan:"
	printf '  %s\n' "${found_after[@]}"
	exit 0
elif [[ -d "/sys/bus/pci/devices/${endpoint}" ]]; then
	rebind_endpoint="${endpoint}"
	mapfile -t found_after < <(printf "%s\n" "${endpoint}")
else
	mapfile -t found_after < <(find_by_id "${id_pat}")
fi

if (( ${#found_after[@]} == 0 )); then
	echo "No ${id_pat} device found after reset+rescan."
	echo "Bitstream may expose a different PCI ID; verify with: lspci -Dnn | grep -i 10ee"
	echo "If still missing, try a full cold power cycle (AC off for ~30s)."
	exit 2
fi

echo "Found device(s) after rescan:"
printf '  %s\n' "${found_after[@]}"

if [[ -z "${rebind_endpoint}" ]]; then
	if (( ${#found_after[@]} == 1 )); then
		rebind_endpoint="${found_after[0]}"
	elif (( allow_first_match == 1 )); then
		rebind_endpoint="${found_after[0]}"
		echo "Warning: multiple matches after rescan, rebinding first: ${rebind_endpoint}" >&2
	else
		echo "Warning: multiple matches after rescan, skip auto-rebind." >&2
		echo "Pass explicit BDF or use --allow-first-match." >&2
	fi
fi

if [[ -n "${saved_driver}" && -n "${rebind_endpoint}" ]]; then
	if [[ -L "/sys/bus/pci/devices/${rebind_endpoint}/driver" ]]; then
		current_driver="$(basename "$(readlink -f "/sys/bus/pci/devices/${rebind_endpoint}/driver")")"
		if [[ "${current_driver}" == "${saved_driver}" ]]; then
			echo "Driver already bound: ${saved_driver} on ${rebind_endpoint}"
			exit 0
		fi
		echo "Warning: ${rebind_endpoint} already bound to ${current_driver}; not forcing rebind." >&2
	elif [[ -e "/sys/bus/pci/drivers/${saved_driver}/bind" ]]; then
		echo "Rebinding driver : ${saved_driver} -> ${rebind_endpoint}"
		echo "${rebind_endpoint}" > "/sys/bus/pci/drivers/${saved_driver}/bind"
	else
		echo "Warning: saved driver '${saved_driver}' has no bind node; skip rebind." >&2
	fi
fi
