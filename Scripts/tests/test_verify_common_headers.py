#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""提出用補助ヘッダーの展開・重複・依存追跡を確認する。"""
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_verify as verify


class CommonHeadersTest(unittest.TestCase):
    def test_nested_headers_and_dependency_hashes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            common = root / 'Verify/current/common'
            common.mkdir(parents=True)
            (common / 'a.hpp').write_text('#include "Verify/current/common/b.hpp"\nint a;\n')
            (common / 'b.hpp').write_text('#include <cstdint>\nint b;\n')
            source = '#include "Verify/current/common/a.hpp"\n#include "Verify/current/common/b.hpp"\nint main() {}\n'
            hashes = {}
            with patch.object(verify, 'ROOT', root):
                result = verify.expand_common(source, hashes)
                self.assertEqual(hashes, verify.common_source_hashes(source))
                self.assertEqual(result.count('int b;'), 1)
                self.assertEqual(result.count('int a;'), 1)
                self.assertEqual(len(hashes), 2)
                (common / 'b.hpp').write_text('int changed;\n')
                self.assertNotEqual(hashes, verify.common_source_hashes(source))
            self.assertNotIn('#include "', result)

    def test_legacy_io_expansion_preserves_bytes(self):
        source = verify.FAST_IO_INCLUDE + '\n\nint main() {}\n'
        expected = source.replace(verify.FAST_IO_INCLUDE,
                                  '// Verify I/O: ' + verify.FAST_IO + '\n' +
                                  (verify.ROOT / verify.FAST_IO).read_text())
        self.assertEqual(verify.expand_common(source), expected)

    def test_unrecognized_headers_remain_for_rejection(self):
        source = '#include "Verify/current/common/../private.hpp"\n#include "outside.hpp"\n'
        self.assertEqual(verify.expand_common(source), source)


if __name__ == '__main__':
    unittest.main()
