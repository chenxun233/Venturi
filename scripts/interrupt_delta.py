#!/usr/bin/env python3
import argparse
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


DEFAULT_TCPREPLAY_COMMAND = [
    "sudo",
    "taskset",
    "-c",
    "12-14",
    "tcpreplay",
    "-i",
    "enp6s0f1",
    "--pps",
    "10000",
    "--loop=4000",
    "market_data/ONE_MSG_ONE_FRAME.pcap",
]


@dataclass(frozen=True)
class InterruptRow:
    counts: list[int]
    description: str


@dataclass(frozen=True)
class InterruptSnapshot:
    cpus: list[str]
    rows: dict[str, InterruptRow]


def _parse_counter_snapshot(text, source_name, keep_description):
    lines = [line.rstrip() for line in text.splitlines() if line.strip()]
    if not lines:
        raise ValueError(f"{source_name} snapshot is empty")

    cpus = lines[0].split()
    if not cpus or not all(cpu.startswith("CPU") for cpu in cpus):
        raise ValueError(f"first {source_name} line does not contain CPU headers")

    rows = {}
    cpu_count = len(cpus)
    for line in lines[1:]:
        fields = line.split()
        if len(fields) < cpu_count + 1:
            continue

        name = fields[0]
        count_fields = fields[1 : cpu_count + 1]
        if not all(field.isdigit() for field in count_fields):
            continue

        description = " ".join(fields[cpu_count + 1 :]) if keep_description else ""
        rows[name] = InterruptRow([int(field) for field in count_fields], description)

    return InterruptSnapshot(cpus, rows)


def parse_interrupts(text):
    return _parse_counter_snapshot(text, "/proc/interrupts", True)


def parse_softirqs(text):
    return _parse_counter_snapshot(text, "/proc/softirqs", False)


def compute_deltas(before, after):
    if before.cpus != after.cpus:
        raise ValueError("CPU headers changed between snapshots")

    rows = {}
    zero_counts = [0] * len(after.cpus)
    for name, after_row in after.rows.items():
        before_counts = before.rows.get(name, InterruptRow(zero_counts, "")).counts
        rows[name] = InterruptRow(
            [after_count - before_count for before_count, after_count in zip(before_counts, after_row.counts)],
            after_row.description,
        )

    return InterruptSnapshot(after.cpus, rows)


def parse_cpu_filter(value):
    cpus = set()
    if not value:
        return cpus

    for part in value.split(","):
        part = part.strip()
        if not part:
            continue
        if "-" in part:
            start_text, end_text = part.split("-", 1)
            start = int(start_text)
            end = int(end_text)
            if end < start:
                raise ValueError(f"invalid CPU range: {part}")
            cpus.update(range(start, end + 1))
        else:
            cpus.add(int(part))
    return cpus


def _selected_indices(cpus, cpu_filter):
    selected = []
    for index, cpu in enumerate(cpus):
        cpu_number = int(cpu.removeprefix("CPU"))
        if not cpu_filter or cpu_number in cpu_filter:
            selected.append(index)
    return selected


def format_deltas(deltas, cpu_filter=None, show_zero=False):
    selected = _selected_indices(deltas.cpus, cpu_filter or set())
    if not selected:
        raise ValueError("CPU filter did not match any CPU columns")

    name_width = max([4] + [len(name) for name in deltas.rows])
    count_width = max(8, max(len(cpu) for cpu in deltas.cpus))
    lines = [" " * (name_width + 1) + " ".join(f"{deltas.cpus[index]:>{count_width}}" for index in selected) + "  description"]

    for name, row in deltas.rows.items():
        selected_counts = [row.counts[index] for index in selected]
        if not show_zero and all(count == 0 for count in selected_counts):
            continue
        counts = " ".join(f"{count:>{count_width}}" for count in selected_counts)
        lines.append(f"{name:<{name_width}} {counts}  {row.description}".rstrip())

    if len(lines) == 1:
        lines.append("No interrupt deltas matched the selected CPUs.")

    return "\n".join(lines)


def normalize_command(command):
    if command and command[0] == "--":
        command = command[1:]
    return command or DEFAULT_TCPREPLAY_COMMAND


def read_snapshot(path):
    return Path(path).read_text(encoding="utf-8")


def build_parser():
    parser = argparse.ArgumentParser(
        description="Snapshot /proc/interrupts and /proc/softirqs before and after a command, then print per-CPU deltas.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "command",
        nargs=argparse.REMAINDER,
        help="Command to run. Defaults to the tcpreplay command used for full capture.",
    )
    parser.add_argument(
        "--interrupts",
        default="/proc/interrupts",
        help="Interrupts file to snapshot. Use an empty string to disable.",
    )
    parser.add_argument(
        "--softirqs",
        default="/proc/softirqs",
        help="Softirqs file to snapshot. Use an empty string to disable.",
    )
    parser.add_argument(
        "--cpus",
        default="2-9",
        help="CPU columns to print, for example 2-9 or 0,12-14. Use an empty string to print all CPUs.",
    )
    parser.add_argument(
        "--show-zero",
        action="store_true",
        help="Print rows whose selected CPU deltas are all zero.",
    )
    return parser


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        cpu_filter = parse_cpu_filter(args.cpus)
        command = normalize_command(args.command)
        before_interrupts = parse_interrupts(read_snapshot(args.interrupts)) if args.interrupts else None
        before_softirqs = parse_softirqs(read_snapshot(args.softirqs)) if args.softirqs else None
        print("Running:", " ".join(command), file=sys.stderr)
        result = subprocess.run(command, check=False)
        after_interrupts = parse_interrupts(read_snapshot(args.interrupts)) if args.interrupts else None
        after_softirqs = parse_softirqs(read_snapshot(args.softirqs)) if args.softirqs else None

        if before_interrupts is not None and after_interrupts is not None:
            print("/proc/interrupts delta")
            print(format_deltas(compute_deltas(before_interrupts, after_interrupts), cpu_filter=cpu_filter, show_zero=args.show_zero))
        if before_softirqs is not None and after_softirqs is not None:
            if before_interrupts is not None:
                print()
            print("/proc/softirqs delta")
            print(format_deltas(compute_deltas(before_softirqs, after_softirqs), cpu_filter=cpu_filter, show_zero=args.show_zero))
        return result.returncode
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
