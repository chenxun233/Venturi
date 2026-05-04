import importlib.util
import unittest
from pathlib import Path

MODULE_PATH = Path(__file__).resolve().parents[1] / "scripts" / "runtime_diagnostics.py"
spec = importlib.util.spec_from_file_location("runtime_diagnostics", MODULE_PATH)
runtime_diagnostics = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runtime_diagnostics)


class RuntimeDiagnosticsTest(unittest.TestCase):
    def test_filter_thread_lines_keeps_header_and_matching_processes(self):
        text = """    PID     TID PSR COMMAND
      1       1  17 systemd
    100     101   2 Venturi
    200     201  12 tcpreplay
    300     301  10 dummy_server
"""

        output = runtime_diagnostics.filter_thread_lines(text, ["Venturi", "dummy_server", "tcpreplay"])

        self.assertIn("PID", output)
        self.assertIn("Venturi", output)
        self.assertIn("tcpreplay", output)
        self.assertIn("dummy_server", output)
        self.assertNotIn("systemd", output)

    def test_build_sections_groups_commands_by_category(self):
        sections = runtime_diagnostics.build_sections("enp6s0f1")

        names = [section.name for section in sections]
        self.assertIn("NIC queue and RSS configuration", names)
        self.assertIn("NIC RPS and XPS CPU masks", names)
        self.assertIn("IRQ affinity", names)
        self.assertIn("Runtime thread placement", names)


if __name__ == "__main__":
    unittest.main()
