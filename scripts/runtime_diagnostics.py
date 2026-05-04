#!/usr/bin/env python3
import argparse
import glob
import subprocess
from dataclasses import dataclass
from pathlib import Path


DEFAULT_PROCESS_PATTERNS = ["Venturi", "dummy_server", "tcpreplay"]


@dataclass(frozen=True)
class Probe:
    label: str
    command: list[str] | None = None
    glob_pattern: str | None = None
    filter_processes: bool = False


@dataclass(frozen=True)
class Section:
    name: str
    probes: list[Probe]


def filter_thread_lines(text, process_patterns):
    lines = text.splitlines()
    if not lines:
        return ""

    output = [lines[0]]
    for line in lines[1:]:
        if any(pattern in line for pattern in process_patterns):
            output.append(line)
    return "\n".join(output) + "\n"


def build_sections(interface):
    return [
        Section(
            "NIC queue and RSS configuration",
            [
                Probe(f"ethtool -x {interface}", ["ethtool", "-x", interface]),
                Probe(f"ethtool -l {interface}", ["ethtool", "-l", interface]),
            ],
        ),
        Section(
            "NIC RPS and XPS CPU masks",
            [
                Probe("RX RPS CPU masks", glob_pattern=f"/sys/class/net/{interface}/queues/rx-*/rps_cpus"),
                Probe("RX RPS flow counts", glob_pattern=f"/sys/class/net/{interface}/queues/rx-*/rps_flow_cnt"),
                Probe("TX XPS CPU masks", glob_pattern=f"/sys/class/net/{interface}/queues/tx-*/xps_cpus"),
            ],
        ),
        Section(
            "IRQ affinity",
            [
                Probe("IRQ smp_affinity_list", glob_pattern="/proc/irq/*/smp_affinity_list"),
            ],
        ),
        Section(
            "Runtime thread placement",
            [
                Probe(
                    "Venturi/dummy_server/tcpreplay thread placement",
                    ["ps", "-eLo", "pid,tid,psr,comm"],
                    filter_processes=True,
                ),
            ],
        ),
    ]


def run_command(command):
    try:
        result = subprocess.run(command, check=False, text=True, capture_output=True)
    except OSError as exc:
        return f"<unavailable: {exc}>\n"

    output = result.stdout
    if result.stderr:
        output += result.stderr
    if result.returncode != 0:
        output += f"<exit status: {result.returncode}>\n"
    return output or "<no output>\n"


def read_file(path):
    try:
        return Path(path).read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        return f"<unavailable: {exc}>\n"


def collect_glob(pattern):
    paths = sorted(glob.glob(pattern))
    if not paths:
        return f"<no matches: {pattern}>\n"

    chunks = []
    for path in paths:
        chunks.append(f"{path}: {read_file(path).strip()}")
    return "\n".join(chunks) + "\n"


def run_probe(probe, process_patterns):
    if probe.command is not None:
        output = run_command(probe.command)
        if probe.filter_processes:
            output = filter_thread_lines(output, process_patterns)
        return output
    if probe.glob_pattern is not None:
        return collect_glob(probe.glob_pattern)
    return "<invalid probe>\n"


def print_sections(sections, process_patterns):
    for section in sections:
        print(f"## {section.name}")
        for probe in section.probes:
            print(f"\n### {probe.label}")
            print(run_probe(probe, process_patterns).rstrip())
        print()


def build_parser():
    parser = argparse.ArgumentParser(
        description="Collect categorized runtime diagnostics for NIC steering, IRQ affinity, and benchmark thread placement.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--interface", default="enp6s0f1", help="Network interface to inspect.")
    parser.add_argument(
        "--process",
        action="append",
        default=[],
        help="Process/thread name pattern to include in the runtime thread placement section.",
    )
    return parser


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)
    process_patterns = args.process or DEFAULT_PROCESS_PATTERNS
    print_sections(build_sections(args.interface), process_patterns)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
