# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import json
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import update_verify_status as status


class SubmissionPolicyTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.write("Scripts/library_layout.json", json.dumps(["DataStructure"]))
        self.write("DataStructure/fixture.cpp", "int library_value = 1;\n")
        self.write("Verify/current/common/fast_io.hpp", "int io_value = 2;\n")
        self.driver = "Verify/current/drivers/fixture.cpp"
        self.write(self.driver, status.prepare_verify.FAST_IO_INCLUDE + "\nint main(){return library_value+io_value;}\n")
        self.entries = [dict(id=name, libraries=["DataStructure/fixture.cpp"], driver=self.driver,
                            problem_url=url, samples=[])
                        for name, url in [("old", "https://atcoder.jp/contests/abc001/tasks/abc001_a"),
                                          ("primary", "https://judge.yosupo.jp/problem/fixture"),
                                          ("other", "https://onlinejudge.u-aizu.ac.jp/problems/GRL_4_B")]]
        self.write("Verify/current/groups/fixture.json", json.dumps(dict(entries=self.entries)))
        self.policy_path = self.root / "Verify/current/submission_policy.json"
        self.journal_path = self.root / "Verify/current/judgements.json"

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def source(self, identity="old"):
        entry = next(e for e in self.entries if e["id"] == identity)
        with status.repository(self.root):
            return status.prepare_verify.render(entry)[0]

    def rules(self, rules):
        self.policy_path.write_text(json.dumps(dict(format=1, entries=rules)))

    def doc_rule(self, identity="old"):
        return dict(id=identity, kind="documentation_only", source_sha256=status.digest(self.source(identity)),
                    reason="User explicitly exempts this exact source.")

    def reference_rule(self, identity="old", replacements=None):
        return dict(id=identity, kind="reference", replacements=replacements or ["primary"],
                    reason="Preferred online Verify replaces this historical task.")

    def row(self, identity="old"):
        return next(e for e in status.build_status(self.root)["entries"] if e["id"] == identity)

    def old_ac_then_comment_change(self):
        record = dict(id="old", status="AC", url="https://atcoder.jp/contests/abc001/submissions/123",
                      submission_sha256=status.digest(self.source()))
        self.journal_path.write_text(json.dumps(dict(entries=[record], immutable="history")))
        path = self.root / self.driver
        path.write_text(path.read_text() + "// explanation changed\n")
        return self.journal_path.read_bytes()

    def test_missing_or_empty_policy_preserves_default_status(self):
        self.old_ac_then_comment_change()
        before = status.build_status(self.root)
        self.rules([])
        self.assertEqual(status.build_status(self.root), before)
        self.assertTrue(self.row()["submission_required"])

    def test_documentation_exemption_is_sha_limited_and_never_promotes_ac(self):
        journal = self.old_ac_then_comment_change()
        self.rules([self.doc_rule()])
        row = self.row()
        self.assertEqual(row["status"], "source_changed")
        self.assertFalse(row["submission_required"])
        self.assertEqual(row["judgement"]["submission_sha256"], json.loads(journal)["entries"][0]["submission_sha256"])
        self.assertEqual(self.journal_path.read_bytes(), journal)
        path = self.root / self.driver
        path.write_text(path.read_text().replace("return library_value+io_value", "return 9"))
        self.assertTrue(self.row()["submission_required"])
        self.assertNotIn("submission_policy", self.row())

    def test_no_ac_record_is_created_by_exemption(self):
        self.rules([self.doc_rule()])
        row = self.row()
        self.assertEqual(row["status"], "unsubmitted")
        self.assertFalse(row["submission_required"])
        self.assertNotIn("judgement", row)
        self.assertFalse(self.journal_path.exists())

    def test_url_annotation_changes_also_expire_exact_sha_exemption(self):
        self.rules([self.doc_rule()])
        self.entries[0]["submission_urls"] = ["https://atcoder.jp/contests/abc001/submissions/123"]
        self.write("Verify/current/groups/fixture.json", json.dumps(dict(entries=self.entries)))
        self.assertTrue(self.row()["submission_required"])
        self.assertNotIn("submission_policy", self.row())

    def test_exact_real_ac_remains_real_ac(self):
        self.journal_path.write_text(json.dumps(dict(entries=[dict(id="old", status="AC",
            url="https://atcoder.jp/contests/abc001/submissions/123", submission_sha256=status.digest(self.source()))])))
        self.rules([self.doc_rule()])
        self.assertEqual(self.row()["status"], "current_ac")
        self.assertFalse(self.row()["submission_required"])

    def test_reference_keeps_status_but_exempts_resubmission(self):
        journal = self.old_ac_then_comment_change()
        self.rules([self.reference_rule()])
        row = self.row()
        self.assertEqual(row["status"], "source_changed")
        self.assertEqual(row["submission_policy"]["replacements"], ["primary"])
        self.assertFalse(row["submission_required"])
        self.assertTrue(self.row("primary")["submission_required"])
        self.assertEqual(self.journal_path.read_bytes(), journal)

    def test_presentations_distinguish_exempt_and_reference_from_current_ac(self):
        self.old_ac_then_comment_change()
        self.rules([self.doc_rule(), self.reference_rule("other")])
        data = status.build_status(self.root)
        self.assertEqual(data["counts"]["current_ac"], 0)
        self.assertIn("説明変更のみ・再提出不要", status.status_markdown(data))
        self.assertIn("参考検証・移行先あり", status.status_markdown(data))
        self.assertIn("再提出免除: 1件", status.readme_summary(data))
        self.assertIn("参考検証: 1件", status.readme_summary(data))

    def test_case_insensitive_sha_and_independent_action_output(self):
        rule = self.doc_rule()
        rule["source_sha256"] = rule["source_sha256"].upper()
        self.rules([rule])
        self.assertFalse(self.row()["submission_required"])
        parsed = status.submission_policy(self.root, self.entries)
        self.assertTrue(status.submission_action(parsed["old"], status.digest(self.source()).upper()))
        self.assertEqual(status.submission_action(parsed["old"], "invalid"), {})
        rule = self.reference_rule()
        action = status.submission_action(rule, status.digest(self.source()))
        action["replacements"].append("other")
        self.assertEqual(rule["replacements"], ["primary"])

    def test_invalid_format_and_duplicate_json_keys(self):
        for text in ['[]', '{}', '{"format":true,"entries":[]}', '{"format":2,"entries":[]}',
                     '{"format":1,"entries":{}}', '{"format":1,"format":1,"entries":[]}',
                     '{"format":1,"entries":[{"id":"old","id":"primary"}]}']:
            with self.subTest(text=text):
                self.policy_path.write_text(text)
                with self.assertRaises(ValueError):
                    status.submission_policy(self.root, self.entries)

    def test_invalid_ids_kinds_reasons_and_hashes(self):
        rules = [None, dict(self.doc_rule(), id="unknown"), dict(self.doc_rule(), kind="unknown")]
        rules += [dict(self.doc_rule(), id=v) for v in [None, 1, [], {}, "bad/id"]]
        rules += [dict(self.doc_rule(), reason=v) for v in [None, "", " \n", False]]
        rules += [dict(self.doc_rule(), source_sha256=v) for v in [None, "", "a" * 63, "g" * 64, True]]
        for rule in rules:
            with self.subTest(rule=rule):
                self.rules([rule])
                with self.assertRaises(ValueError):
                    status.submission_policy(self.root, self.entries)
        self.rules([self.doc_rule(), self.doc_rule()])
        with self.assertRaises(ValueError):
            status.submission_policy(self.root, self.entries)

    def test_reference_requires_real_unique_nonself_primary_ids(self):
        for replacements in [None, [], "primary", ["unknown"], ["old"], ["primary", "primary"], [[]], [1]]:
            rule = self.reference_rule()
            rule["replacements"] = replacements
            self.rules([rule])
            with self.subTest(replacements=replacements), self.assertRaises(ValueError):
                status.submission_policy(self.root, self.entries)
        for rules in [[self.reference_rule(), self.reference_rule("primary", ["old"])],
                      [self.reference_rule(), self.reference_rule("primary", ["other"]), self.reference_rule("other", ["old"])],
                      [self.reference_rule(), self.reference_rule("primary", ["other"])]]:
            self.rules(rules)
            with self.assertRaises(ValueError):
                status.submission_policy(self.root, self.entries)


if __name__ == "__main__":
    unittest.main()
