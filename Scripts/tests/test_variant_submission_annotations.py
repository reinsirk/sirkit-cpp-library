#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""通常版のURLを転記せず、その配布版へ追記したURLだけを維持する。"""
from pathlib import Path
import json
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_variant_verify as variants
import prepare_verify as normal


class VariantAnnotationsTest(unittest.TestCase):
    def test_source_provenance_and_preservation(self):
        old_url = 'https://judge.yosupo.jp/submission/100'
        actual_url = 'https://judge.yosupo.jp/submission/200'
        entry = {'id': 'probe', 'libraries': ['Math/Probe.cpp'],
                 'driver': 'Verify/current/drivers/probe.cpp',
                 'problem_url': 'https://judge.yosupo.jp/problem/aplusb',
                 'submission_urls': [old_url]}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            driver = root / entry['driver']
            driver.parent.mkdir(parents=True)
            driver.write_text(normal.FAST_IO_INCLUDE + '\nint main() {}\n')
            io = root / normal.FAST_IO
            io.parent.mkdir(parents=True)
            io.write_text('#include <cstdint>\n')
            for prefix in ('Variants/GCC', 'Variants/Clang', 'Fast/GCC', 'Fast/Clang'):
                source = root / prefix / 'Math/Probe.cpp'
                source.parent.mkdir(parents=True)
                source.write_text('struct Probe {};\n')
                output = root / prefix / 'Verify/current/submissions/probe.cpp'
                output.parent.mkdir(parents=True)
                output.write_text('// Submission: ' + old_url + '\n')
            with patch.object(variants, 'ROOT', root), patch.object(normal, 'ROOT', root), \
                 patch.object(normal, 'collect', return_value=[entry]):
                manifest = variants.prepare()
                self.assertEqual(len(manifest['entries']), 4)
                for item in manifest['entries']:
                    text = (root / item['path']).read_text()
                    self.assertNotIn(old_url, text)
                    self.assertEqual(text.splitlines()[0], '#define SKIP_RUNTIME_CHECKS')
                    self.assertEqual(item['submission_urls'], [])
                submitted = root / 'Fast/GCC/Verify/current/submissions/probe.cpp'
                submitted.write_text(submitted.read_text() + '// Submission: ' + actual_url + '\n')
                manifest = variants.prepare()
                self.assertEqual(submitted.read_text().count('// Submission: '), 1)
                self.assertIn(actual_url, submitted.read_text())
                annotations = json.loads((root / 'Variants/submission_annotations.json').read_text())
                self.assertEqual(annotations, {'Fast/GCC:probe': [actual_url]})
                variants.prepare(check=True)


if __name__ == '__main__':
    unittest.main()
