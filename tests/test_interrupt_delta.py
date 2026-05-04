import importlib.util
import unittest
from pathlib import Path

MODULE_PATH = Path(__file__).resolve().parents[1] / "scripts" / "interrupt_delta.py"
spec = importlib.util.spec_from_file_location("interrupt_delta", MODULE_PATH)
interrupt_delta = importlib.util.module_from_spec(spec)
spec.loader.exec_module(interrupt_delta)


class InterruptDeltaTest(unittest.TestCase):
    def test_parse_interrupts_keeps_irq_name_cpu_counts_and_description(self):
        text = """           CPU0       CPU1       CPU12\n  24:         10         20          3  IR-PCI-MSI 524288-edge      enp6s0f1-TxRx-0\n NMI:          1          2          3   Non-maskable interrupts\n"""

        snapshot = interrupt_delta.parse_interrupts(text)

        self.assertEqual(snapshot.cpus, ["CPU0", "CPU1", "CPU12"])
        self.assertEqual(snapshot.rows["24:"].counts, [10, 20, 3])
        self.assertEqual(snapshot.rows["24:"].description, "IR-PCI-MSI 524288-edge enp6s0f1-TxRx-0")
        self.assertEqual(snapshot.rows["NMI:"].counts, [1, 2, 3])
        self.assertEqual(snapshot.rows["NMI:"].description, "Non-maskable interrupts")

    def test_compute_deltas_subtracts_matching_rows_and_preserves_new_rows(self):
        before = interrupt_delta.parse_interrupts(
            """           CPU0       CPU1\n  24:         10         20  device-a\n"""
        )
        after = interrupt_delta.parse_interrupts(
            """           CPU0       CPU1\n  24:         15         25  device-a\n  25:          7          9  device-b\n"""
        )

        deltas = interrupt_delta.compute_deltas(before, after)

        self.assertEqual(deltas.cpus, ["CPU0", "CPU1"])
        self.assertEqual(deltas.rows["24:"].counts, [5, 5])
        self.assertEqual(deltas.rows["25:"].counts, [7, 9])

    def test_format_deltas_hides_zero_rows_by_default_and_can_filter_cpus(self):
        deltas = interrupt_delta.InterruptSnapshot(
            cpus=["CPU0", "CPU1", "CPU12"],
            rows={
                "24:": interrupt_delta.InterruptRow([0, 0, 0], "quiet"),
                "25:": interrupt_delta.InterruptRow([1, 0, 3], "active"),
            },
        )

        output = interrupt_delta.format_deltas(deltas, cpu_filter={12}, show_zero=False)

        self.assertIn("CPU12", output)
        self.assertNotIn("CPU0", output)
        self.assertNotIn("24:", output)
        self.assertIn("25:", output)
        self.assertIn("active", output)
        self.assertIn("3", output)

    def test_parser_defaults_to_interrupt_cpu_range_2_to_9(self):
        parser = interrupt_delta.build_parser()

        args = parser.parse_args([])

        self.assertEqual(args.cpus, "2-9")

    def test_parse_softirqs_uses_same_snapshot_shape_without_description(self):
        text = """                    CPU0       CPU1       CPU2
          TIMER:         10         20         30
         NET_TX:          1          0          3
"""

        snapshot = interrupt_delta.parse_softirqs(text)

        self.assertEqual(snapshot.cpus, ["CPU0", "CPU1", "CPU2"])
        self.assertEqual(snapshot.rows["TIMER:"].counts, [10, 20, 30])
        self.assertEqual(snapshot.rows["TIMER:"].description, "")
        self.assertEqual(snapshot.rows["NET_TX:"].counts, [1, 0, 3])

    def test_parser_defaults_to_capture_interrupts_and_softirqs(self):
        parser = interrupt_delta.build_parser()

        args = parser.parse_args([])

        self.assertTrue(args.interrupts)
        self.assertTrue(args.softirqs)

    def test_normalize_command_removes_separator_token(self):
        command = interrupt_delta.normalize_command(["--", "true"])

        self.assertEqual(command, ["true"])

    def test_normalize_command_uses_default_when_empty(self):
        command = interrupt_delta.normalize_command([])

        self.assertEqual(command, interrupt_delta.DEFAULT_TCPREPLAY_COMMAND)


if __name__ == "__main__":
    unittest.main()
