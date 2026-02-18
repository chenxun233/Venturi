# PCIe Runtime Reset Recovery (No Reboot)

## Problem Summary

Observed behavior:
- `echo 1 > /sys/bus/pci/devices/0000:05:00.0/remove`
- `echo 1 > /sys/bus/pci/rescan`
- Device does not come back in `lspci`

But this works:
- Reset upstream bridge `0000:00:1d.0`
- Then rescan

## Why `remove + rescan` Is Not Enough

`remove` only removes the endpoint from the Linux PCI device tree.
`rescan` only re-enumerates what is electrically/link-wise present.

If the endpoint link does not retrain after runtime reconfiguration, Linux has nothing to rediscover.

Resetting the upstream bridge forces downstream link retrain, which is why parent reset can recover the FPGA endpoint.

## Topology Example (This Host)

From `lspci -t`:

- `+-1d.0-[05]--`

This means:
- Parent bridge: `0000:00:1d.0`
- Downstream bus: `05`
- Previous endpoint `0000:05:00.0` lives under that parent

When `0000:05:00.0` disappears, parent is still present and can be reset.

## Confirmed ILA Clue

In a captured run after programming:
- `user_lnk_up = 1`
- `cfg_phy_link_down = 0`
- no PCIe error bits

So PCIe core can be healthy while Linux enumeration still needs correct reset sequence.

## Recommended Recovery Command

Use the project script:

```bash
sudo scripts/reset_pcie.sh 0000:05:00.0
```

The script now handles both cases:
- Endpoint exists: unbind -> remove -> parent reset/hot reset -> rescan -> optional rebind
- Endpoint already missing: find parent by bus -> parent reset/hot reset -> rescan

## Manual Recovery (If Needed)

```bash
PARENT=0000:00:1d.0

echo 1 | sudo tee /sys/bus/pci/devices/$PARENT/reset
sleep 1
echo 1 | sudo tee /sys/bus/pci/rescan

lspci -Dnn | grep -Ei "10ee|xilinx|fpga"
lspci -t
```

## Common Pitfalls

- Typo path with comma: `/sys/bus/pci/rescan,` is invalid.
- Empty parent variable causes `/sys/bus/pci/devices//reset` permission/path errors.
- BDF may change after rescan; search by vendor/device if needed.

## Practical Guidance

- Always prefer explicit BDF in scripts.
- If runtime update fails repeatedly but cold boot works, focus on warm/hot reset behavior and link retrain path.
- Keep ILA trigger on `cfg_pl_status_change` and observe LTSSM transitions during reset/rescan.
