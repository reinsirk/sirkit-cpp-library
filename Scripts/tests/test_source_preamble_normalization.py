# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import copy
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import update_verify_status as status
import record_library_checker_submission as importer


HEADER = (b'#define SKIP_RUNTIME_CHECKS\n#define NDEBUG\n'
          b'#if defined(__GNUC__) && !defined(__clang__)\n'
          b'#pragma GCC optimize("O3", "unroll-loops")\n#endif\n'
          b'// Verify: https://judge.yosupo.jp/problem/example\n'
          b'\n// Library: DataStructure/example.cpp\n')
BODY = (b'// SPDX-License-Identifier: CC0-1.0\n#include <cstdint>\n'
        b'namespace /* marker */ sirkit {\nconstexpr int value = 7;\n}\n'
        b'int main() { return sirkit::value; }\n')


def record_for(source):
    proof = status.preamble_source_proof(source)
    return dict(status="AC", uploaded_source_sha256=status.digest(source),
                submission_sha256="a" * 64, source_normalization_proofs=[proof])


def annotate(source):
    return source.replace(b'// Verify: ', b'// Submission: https://judge.yosupo.jp/submission/8\n// Verify: ', 1)


class PreambleNormalizationTests(unittest.TestCase):
    def test_generated_prefix_keeps_all_body_bytes(self):
        source = HEADER + BODY
        normalized = status.normalized_preamble_source(source)
        expected = HEADER.replace(b'// Verify: https://judge.yosupo.jp/problem/example\n', b'').replace(
            b'// Library: DataStructure/example.cpp\n', b'') + BODY
        self.assertEqual(normalized, expected)
        self.assertEqual(status.source_match(record_for(source), annotate(source)),
                         "generated_preamble_metadata_normalized")

    def test_first_library_label_is_metadata_but_later_labels_are_body(self):
        source = HEADER + BODY + b'// Library: Internal/second.cpp\n'
        changed = source.replace(b'// Library: DataStructure/example.cpp', b'// Library: DataStructure/renamed.cpp')
        self.assertEqual(status.normalized_preamble_source(source), status.normalized_preamble_source(changed))
        self.assertIsNone(status.source_match(record_for(source), source.replace(b'Internal/second', b'Internal/third')))

    def test_every_real_body_change_is_rejected_even_with_legacy_proof(self):
        source = HEADER + BODY
        record = record_for(source)
        record.update(source_normalization="submission-metadata-v1",
                      normalized_source_sha256=status.digest(status.normalized_source(source)))
        for changed in [source.replace(b'value = 7', b'value = 8'), source + b'\n',
                        source + b'// Submission: forged body metadata\n',
                        source.replace(b'// SPDX-License-Identifier: CC0-1.0', b'// another license'),
                        source.replace(b'#include <cstdint>', b'#include <cstdint>  '),
                        source.replace(b'namespace ', b'\nnamespace '), source[:-1]]:
            with self.subTest(changed=changed[-100:]):
                self.assertIsNone(status.source_match(record, changed))

    def test_each_body_byte_is_part_of_the_match(self):
        source = HEADER + BODY
        record = record_for(source)
        for position in range(len(HEADER), len(source)):
            replacement = b"@" if source[position:position + 1] != b"@" else b"$"
            changed = source[:position] + replacement + source[position + 1:]
            with self.subTest(position=position):
                self.assertIsNone(status.source_match(record, changed))

    def test_fake_metadata_inside_raw_strings_and_block_comments_is_opaque(self):
        for body in [b'const char* s = R"tag(\n// Submission: literal\n// Verify: literal\n)tag";\n',
                     b'/*\n// Submission: literal\n// Library: literal\n*/\nint main() {}\n',
                     b'// Submission: a library preamble comment\n#include <cstdint>\n']:
            source = HEADER + body
            self.assertEqual(status.source_match(record_for(source), annotate(source)),
                             "generated_preamble_metadata_normalized")
            self.assertIsNone(status.source_match(record_for(source), source.replace(b'literal', b'changed'))
                              if b'literal' in source else status.source_match(record_for(source), source + b'\n'))

    def test_crlf_without_raw_is_the_only_body_byte_normalization(self):
        source = HEADER + BODY
        uploaded = source.replace(b'\n', b'\r\n')
        self.assertEqual(status.source_match(record_for(uploaded), annotate(source)),
                         "generated_preamble_metadata_normalized")
        self.assertIsNone(status.source_match(record_for(uploaded), annotate(source).replace(b'value = 7', b'value = 7 ')))

    def test_crlf_with_raw_token_is_rejected_but_lf_raw_body_is_allowed(self):
        source = HEADER + b'const char* value = R"(a\nb)";\n'
        self.assertIsNotNone(status.preamble_source_proof(source))
        self.assertIsNone(status.preamble_source_proof(source.replace(b'\n', b'\r\n')))
        self.assertIsNone(status.source_match(record_for(source), annotate(source).replace(b'\n', b'\r\n')))

    def test_line_sensitive_and_preprocessing_hazards_are_conservative(self):
        for body in [b'int x = __LINE__;\n', b'int x = __builtin_LINE();\n',
                     b'const char* file = __FILE__;\n', b'#embed "main.cpp"\n',
                     b'auto x = __builtin_source_location();\n', b'#include <source_location>\n',
                     b'#line 10\n', b'# /**/ line 10\n', b'%:line 10\n',
                     b'#define JOIN(a,b) a ## b\n', b'#define X \\\n1\n',
                     b'#define X \\ \t\n1\n', b'// continued comment \\\nint x;\n',
                     b'??/\nint x;\n', b'int x;\rint y;\n']:
            with self.subTest(body=body):
                self.assertIsNone(status.preamble_source_proof(HEADER + body))

    def test_prefix_cannot_hide_macro_raw_or_block_state(self):
        for prelude in [b'#define UNKNOWN 1\n', b'#include <cstdint>\n', b'int x;\n',
                        b'/* comment\n', b'const char* x=R"(\n',
                        b'#if 1\n', b'#define NDEBUG 1\n']:
            with self.subTest(prelude=prelude):
                self.assertIsNone(status.normalized_preamble_source(prelude + HEADER + BODY))
        for prefix in [b'// Verify: x\n#include <cstdint>\n// Library: fake\n',
                       b'// Verify: x\n#define OTHER 1\n// Library: fake\n',
                       b'// Verify: x\n// Verify: y\n// Library: fake\n']:
            self.assertIsNone(status.normalized_preamble_source(prefix + BODY))

    def test_nonmetadata_prefix_and_settings_are_preserved(self):
        source = HEADER + BODY
        for changed in [source.replace(b'"O3"', b'"O2"'),
                        source.replace(b'#define NDEBUG\n', b''),
                        b'// another generated note\n' + source]:
            self.assertIsNone(status.source_match(record_for(source), changed))

    def test_uploaded_hash_is_mandatory_and_bound_to_proof(self):
        source = HEADER + BODY
        record = record_for(source)
        for modified in [dict(record, uploaded_source_sha256="b" * 64),
                         {k: v for k, v in record.items() if k != "uploaded_source_sha256"},
                         dict(record, source_normalization_proofs=[]),
                         dict(record, source_normalization_proofs=[None])]:
            self.assertIsNone(status.source_match(modified, annotate(source)))
        bad = copy.deepcopy(record)
        bad["source_normalization_proofs"][0]["uploaded_source_sha256"] = "c" * 64
        self.assertIsNone(status.source_match(bad, annotate(source)))
        with self.assertRaises(ValueError):
            status.records_from([dict(bad, id="example")])

    def test_importer_handles_body_markers_but_rejects_real_change(self):
        source = HEADER + BODY
        self.assertEqual(importer.compare_sources(source, annotate(source)),
                         "generated_preamble_metadata_normalized")
        with self.assertRaises(ValueError):
            importer.compare_sources(source, annotate(source).replace(b'value = 7', b'value = 9'))

    def test_new_generated_import_never_falls_back_to_v1(self):
        source = HEADER + b'int main() { return 0; }\n'
        for changed in [annotate(source) + b'// Submission: a body comment\n',
                        annotate(source).replace(b'return 0;', b'return 0; '),
                        annotate(source) + b'\n']:
            with self.assertRaises(ValueError):
                importer.compare_sources(source, changed)
        for body in [b'int value = __LINE__;\n', b'#define X \\\n1\n',
                     b'const char* s = R"(a\nb)";\n']:
            source = HEADER + body
            changed = annotate(source)
            if b'R"' in body:
                changed = changed.replace(b'\n', b'\r\n')
            with self.assertRaises(ValueError):
                importer.compare_sources(source, changed)


if __name__ == "__main__":
    unittest.main()
