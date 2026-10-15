"""Regression tests for the Python-facing reader; run with a rebuilt pyfrd."""
from pathlib import Path
import struct
import tempfile
import unittest

from pyfrd import FrdReader


def make_frd(path, binary, modal_only=False, include_modes=True):
    """Small CalculiX-style mesh with sparse node IDs and two eigenmodes."""
    ids = [3, 7, 11, 19]
    fmt = 2 if binary else 1
    with open(path, "wb") as stream:
        def line(text):
            stream.write((text + "\n").encode("ascii"))

        line(f"    2C{len(ids):30d}{fmt:2d}".ljust(74))
        for i, nid in enumerate(ids):
            xyz = (float(i == 1), float(i == 2), float(i == 3))
            if binary:
                stream.write(struct.pack("<i3d", nid, *xyz))
            else:
                line(f" -1{nid:10d}" + "".join(f"{v:12.5E}" for v in xyz))
        if not binary:
            line(" -3")
        line(f"    3C{1:30d}{fmt:2d}".ljust(74))
        if binary:
            stream.write(struct.pack("<8i", 42, 3, 0, 1, *ids))
        else:
            line(f" -1{42:10d}{3:5d}{0:5d}{1:5d}")
            line(" -2" + "".join(f"{nid:10d}" for nid in ids))
            line(" -3")

        blocks = [] if modal_only else [(1, 0, 1.0, "DISP", ["D1", "D2", "D3"])]
        if binary and not modal_only:
            blocks.append((1, 0, 1.0, "SDV", [f"SDV{i}" for i in range(1, 21)]))
        modes = [(2, 1, 43.72906546), (3, 2, 47.88321163)] if include_modes else []
        for step, mode, frequency in modes:
            blocks.append((step, mode, frequency, "DISP", ["D1", "D2", "D3"]))
            if not modal_only:
                blocks.append((step, mode, frequency, "STRESS", ["SXX", "SYY", "SZZ", "SXY", "SZX", "SYZ"]))
        for step, mode, value, field, components in blocks:
            line(f"    1PSTEP{1:26d}{1:12d}{step:12d}".ljust(68))
            if mode:
                for parameter, data in [("GM", "1.000000E+00"), ("GK", "7.549186E+04"),
                                        ("HID", "-1"), ("SUBC", "0"), ("MODE", str(mode))]:
                    line(f"    {'1P' + parameter:<6}{data:>26}".ljust(68))
            analysis = "MODAL" if mode else ""
            line(f"  100C{'L  101':6s}{value:12.10g}{len(ids):12d}{'':20s}{2 if mode else 0:2d}{step:5d}{analysis:10s}{fmt:2d}")
            derived = field == "DISP"
            line(f" -4  {field:8s}{len(components) + derived:5d}{1:5d}")
            for i, component in enumerate(components):
                line(f" -5  {component:8s}{1:5d}{2:5d}{i + 1:5d}{0:5d}")
            if derived:
                line(f" -5  {'ALL':8s}{1:5d}{2:5d}{0:5d}{0:5d}{1:5d}ALL")
            for nid in ids:
                values = [step * 100 + nid + c / 4 for c in range(len(components))]
                if binary:
                    stream.write(struct.pack("<i" + "f" * len(values), nid, *values))
                else:
                    line(f" -1{nid:10d}" + "".join(f"{v:12.5E}" for v in values))
            if not binary:
                line(" -3")
        line("  9999")


class DynamicResultsTest(unittest.TestCase):
    def test_static_and_multiple_modes_ascii_and_binary(self):
        for binary in (False, True):
            with self.subTest(binary=binary), tempfile.TemporaryDirectory() as directory:
                path = Path(directory) / "results.frd"
                make_frd(path, binary)
                reader = FrdReader()
                reader.read(str(path))
                self.assertEqual(sorted(reader.nodes), [3, 7, 11, 19])
                self.assertEqual(reader.elements[42].indices, [3, 7, 11, 19])
                values, metadata = reader.values, reader.metadata
                self.assertEqual(sorted(values), [1, 2, 3])
                self.assertEqual(metadata[1]["DISP"].mode, 0)
                self.assertEqual(metadata[1]["DISP"].value, 1.0)
                self.assertEqual(values[1]["DISP"]["D1"][7], 107)
                self.assertEqual("SDV" in values[1], binary)
                for step, mode, frequency in [(2, 1, 43.72906546), (3, 2, 47.88321163)]:
                    self.assertEqual(sorted(values[step]), ["DISP", "STRESS"])
                    self.assertEqual(sorted(values[step]["DISP"]), ["D1", "D2", "D3"])
                    for field in ("DISP", "STRESS"):
                        self.assertEqual(metadata[step][field].mode, mode)
                        self.assertEqual(metadata[step][field].num_step, step)
                        self.assertEqual(metadata[step][field].analysis, "MODAL")
                        self.assertAlmostEqual(metadata[step][field].value, frequency, places=7)
                    self.assertEqual(values[step]["DISP"]["D2"][7], step * 100 + 7.25)
                    self.assertEqual(values[step]["STRESS"]["SYZ"][19], step * 100 + 20.25)

    def test_displacement_only_and_reader_reuse(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "results.frd"
            make_frd(path, True, modal_only=True)
            reader = FrdReader()
            reader.read(str(path))
            self.assertEqual(sorted(reader.values), [2, 3])
            self.assertEqual(sorted(reader.values[2]), ["DISP"])
            make_frd(path, False, include_modes=False)
            reader.read(str(path))
            self.assertEqual(sorted(reader.values), [1])
            self.assertEqual(sorted(reader.metadata), [1])
            self.assertEqual(reader.metadata[1]["DISP"].mode, 0)
            self.assertEqual(reader.metadata[1]["DISP"].analysis, "")
            with self.assertRaisesRegex(RuntimeError, "Cannot open FRD"):
                reader.read(str(Path(directory) / "missing.frd"))


if __name__ == "__main__":
    unittest.main()
