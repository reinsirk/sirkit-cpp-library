# SPDX-License-Identifier: CC0-1.0
import hashlib
from pathlib import Path
import stat
import struct
import tempfile
import unittest
from unittest.mock import patch
from zipfile import BadZipFile, ZIP_DEFLATED, ZipFile

from Scripts import deterministic_zip as writer


class ArchiveTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.target = self.root / "archive.zip"

    def test_mapping_order_and_exact_payload(self):
        entries = {"日本語/(a).cpp": "文字列\n".encode(), "z.bin": bytes(range(256)), "A": b""}
        writer.write_archive(self.target, entries)
        before = self.target.read_bytes()
        writer.write_archive(self.target, dict(reversed(list(entries.items()))))
        self.assertEqual(before, self.target.read_bytes())
        with ZipFile(self.target) as archive:
            self.assertEqual(archive.namelist(), sorted(entries))
            for info in archive.infolist():
                self.assertEqual(info.date_time, (1980, 1, 1, 0, 0, 0))
                self.assertEqual(info.create_system, 3)
                self.assertEqual(info.external_attr >> 16, stat.S_IFREG | 0o644)
                self.assertEqual(info.compress_type, ZIP_DEFLATED)
                self.assertEqual(archive.read(info), entries[info.filename])
                if info.filename.startswith("日本語"):
                    self.assertTrue(info.flag_bits & 0x800)
        self.assertEqual(set(writer.archive_versions()), {"python", "zlib_compile", "zlib_runtime"})

    def test_payload_changes_digest(self):
        writer.write_archive(self.target, {"x": b"one"})
        before = hashlib.sha256(self.target.read_bytes()).digest()
        writer.write_archive(self.target, {"x": b"two"})
        self.assertNotEqual(before, hashlib.sha256(self.target.read_bytes()).digest())

    def test_bad_paths_preserve_old_archive(self):
        bad = ["", "/absolute", "../x", "a/../x", "./x", "a/./x", "a//b", "a/", "C:/x", "C:x", "a:b", "\\\\server\\x", "a\\b", "a\x00b", "a\nb", "\x7f", "\x85", "x" * 65536]
        self.target.write_bytes(b"existing")
        for name in bad:
            with self.subTest(name=repr(name)), self.assertRaises(ValueError):
                writer.write_archive(self.target, {"valid": b"first", name: b"bad"})
            self.assertEqual(self.target.read_bytes(), b"existing")
        self.assertEqual(list(self.root.iterdir()), [self.target])

    def test_wrong_payload_preserves_output(self):
        self.target.write_bytes(b"existing")
        with self.assertRaises(TypeError):
            writer.write_archive(self.target, {"a": bytearray(b"wrong")})
        self.assertEqual(self.target.read_bytes(), b"existing")

    def test_duplicate_mapping_items(self):
        class Duplicate(dict):
            def items(self):
                return [("a", b"first"), ("a", b"second")]
        self.target.write_bytes(b"existing")
        with self.assertRaises(ValueError):
            writer.write_archive(self.target, Duplicate())
        self.assertEqual(self.target.read_bytes(), b"existing")

    def test_empty_archive(self):
        writer.write_archive(self.target, {})
        first = self.target.read_bytes()
        writer.write_archive(self.target, {})
        self.assertEqual(first, self.target.read_bytes())
        with ZipFile(self.target) as archive:
            self.assertEqual(archive.namelist(), [])

    def test_crc_failure_keeps_old_output(self):
        original = writer.ZipFile
        class CorruptAfterWrite(original):
            def __exit__(self, *args):
                filename, mode = self.filename, self.mode
                result = super().__exit__(*args)
                if mode == "w":
                    file = Path(filename)
                    data = bytearray(file.read_bytes())
                    # Change local and central CRC together; payload remains unchanged.
                    for signature, offset in [(b"PK\x03\x04", 14), (b"PK\x01\x02", 16)]:
                        at = data.index(signature) + offset
                        crc = struct.unpack_from("<I", data, at)[0]
                        struct.pack_into("<I", data, at, crc ^ 1)
                    file.write_bytes(data)
                return result
        self.target.write_bytes(b"existing")
        with patch.object(writer, "ZipFile", CorruptAfterWrite), self.assertRaises(BadZipFile):
            writer.write_archive(self.target, {"a": b"payload"})
        self.assertEqual(self.target.read_bytes(), b"existing")
        self.assertEqual(list(self.root.iterdir()), [self.target])


if __name__ == "__main__":
    unittest.main()
