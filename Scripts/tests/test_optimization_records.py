# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import copy
import json
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import optimization_records as records


class OptimizationRecordsTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.base = Path(self.temporary.name)
        self.root = self.base / "repo"
        for directory in ["Scripts", "DataStructure", "Verify/current/groups", "Verify/current/common", "Verify/current/drivers", "Audit"]:
            (self.root / directory).mkdir(parents=True, exist_ok=True)
        (self.root / "Scripts/library_layout.json").write_text('["DataStructure"]')
        (self.root / "AGENTS.md").write_text("Keep the public API.\n")
        (self.root / "DataStructure/a.cpp").write_text("int library_value = 1;\n")
        (self.root / "DataStructure/b.cpp").write_text("int second_value = 2;\n")
        (self.root / "Verify/current/common/fast_io.hpp").write_text("// input/output fixture\n")
        self.entries = []
        for identity, library, url in [
            ("accepted", "a", "https://judge.yosupo.jp/problem/unionfind"),
            ("shared", "a", "https://judge.yosupo.jp/problem/staticrmq"),
            ("unverified", "b", "https://atcoder.jp/contests/abc001/tasks/abc001_1"),
            ("local", "b", None),
        ]:
            path = f"Verify/current/drivers/{identity}.cpp"
            (self.root / path).write_text('#include "Verify/current/common/fast_io.hpp"\nint main() {return 0;}\n')
            self.entries.append({"id": identity, "libraries": [f"DataStructure/{library}.cpp"],
                                 "driver": path, "problem_url": url})
        self.group = self.root / "Verify/current/groups/test.json"
        self.group.write_text(json.dumps({"entries": self.entries}))
        self.journal = self.root / "Verify/current/judgements.json"
        self.proof = self.root / "Audit/proof.json"
        self.proof.write_text('{"correctness":"passed","timing":[1,2,3]}\n')
        self.write_journal([self.ac_record()])
        self.ledger = self.root / "Verify/optimization/stages"

    def item(self, name="accepted"):
        return next(e for e in records.catalog(self.root) if e["id"] == name)

    def ac_record(self, name="accepted"):
        return {"id": name, "status": "AC", "url": "https://judge.yosupo.jp/submission/123",
                "submission_sha256": self.item(name)["source_sha256"]}

    def write_journal(self, records):
        self.journal.write_text(json.dumps({"entries": records}))

    def screenshot_record(self):
        source = self.item("unverified")["source"]
        return {"id": "unverified", "status": "AC",
                "url": "https://atcoder.jp/contests/abc001/submissions/123",
                "evidence_kind": "user_screenshot", "needs_source_confirmation": True,
                "needs_resubmission": False, "expected_source_sha256": records.digest(source),
                "expected_normalized_source_sha256": records.digest(records.status.normalized_source(source)),
                "expected_source_normalization": "submission-metadata-v1"}

    def record_atcoder_claude(self):
        value = self.receipt("claude", identity="unverified")
        records.record_stage(self.root, self.ledger, self.receipt_file(value))

    def atcoder_row(self):
        return next(row for row in records.queue(self.root, self.ledger)["entries"]
                    if row["id"] == "unverified")

    def receipt(self, stage="codex", *, before=None, identity="accepted"):
        after = self.item(identity)["source_sha256"]
        return {"format": 1, "id": identity, "stage": stage, "status": "complete",
                "input_source_sha256": before or after, "source_sha256": after,
                "changed": bool(before and before != after),
                "scope": "whole_program" if self.item(identity)["library_checker"] else "library",
                "checks": {"correctness": "passed", "performance": "measured"},
                "evidence": [{"path": "Audit/proof.json", "sha256": records.digest(self.proof.read_bytes())}]}

    def receipt_file(self, value, name="receipt.json"):
        path = self.base / name
        path.write_bytes(records.encode(value))
        return path

    def test_default_queue_only_has_accepted_history(self):
        rows = records.queue(self.root, self.ledger)["entries"]
        self.assertEqual([r["id"] for r in rows], ["accepted"])
        self.assertTrue(rows[0]["current_ac"])
        self.assertEqual(rows[0]["next_stage"], "codex")
        self.assertEqual(len(records.queue(self.root, self.ledger, True)["entries"]), 3)

    def test_no_journal_and_corrupt_journal(self):
        self.journal.unlink()
        self.assertEqual(records.queue(self.root, self.ledger)["entries"], [])
        self.write_journal({"not": "an array"})
        with self.assertRaises(ValueError):
            records.catalog(self.root)

    def test_duplicate_record_id_is_rejected(self):
        row = self.ac_record()
        self.write_journal([row, row])
        with self.assertRaises(ValueError):
            records.catalog(self.root)

    def test_orphan_record_is_not_queued(self):
        self.write_journal([self.ac_record(), {"id": "orphan", "status": "AC", "url": "https://judge.yosupo.jp/submission/1"}])
        self.assertEqual(len(records.queue(self.root, self.ledger)["entries"]), 1)

    def test_nested_history_wrapper_is_used(self):
        record = self.ac_record()
        self.write_journal([{"id": "accepted", "status": "WA", "history": [{"record": record}, {"note": "local check"}]}])
        self.assertTrue(self.item()["current_ac"])

    def test_wrong_judge_does_not_qualify(self):
        record = self.ac_record()
        record["url"] = "https://atcoder.jp/contests/abc001/submissions/1"
        self.write_journal([record])
        self.assertFalse(self.item()["historical_ac"])

    def test_uploaded_exact_source_hash_qualifies(self):
        record = self.ac_record()
        record["uploaded_source_sha256"] = record.pop("submission_sha256")
        self.write_journal([record])
        self.assertTrue(self.item()["current_ac"])

    def test_current_hash_claim_is_not_proof(self):
        record = self.ac_record()
        record["current_submission_sha256"] = record.pop("submission_sha256")
        record["matched_current_generation"] = True
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])

    def test_screenshot_confirmation_waits_for_claude_and_never_proves_ac(self):
        record = self.screenshot_record()
        self.write_journal([record])
        row = self.atcoder_row()
        self.assertEqual(row["next_stage"], "claude")
        self.assertTrue(row["submission_required"])
        self.assertNotIn("source_confirmation_required", row)
        self.record_atcoder_claude()
        row = self.atcoder_row()
        self.assertEqual(row["next_stage"], "confirm_source")
        self.assertFalse(row["submission_required"])
        self.assertTrue(row["source_confirmation_required"])
        self.assertFalse(row["current_ac"])
        state = records.status.build_status(self.root)
        current = next(e for e in state["entries"] if e["id"] == "unverified")
        self.assertEqual(current["status"], "source_changed")
        # Only submission metadata changes; expected SHA may match the frozen stage.
        self.entries[2]["submission_urls"] = [record["url"]]
        self.group.write_text(json.dumps({"entries": self.entries}))
        item = self.item("unverified")
        self.assertNotEqual(item["source_sha256"], record["expected_source_sha256"])
        self.assertNotIn(record["expected_source_sha256"], item["accepted_source_aliases"])
        self.assertEqual(self.atcoder_row()["next_stage"], "confirm_source")
        self.assertFalse(self.atcoder_row()["current_ac"])

    def test_screenshot_confirmation_requires_latest_host_and_explicit_flags(self):
        record = self.screenshot_record()
        ordinary = {"id": record["id"], "status": "AC", "url": record["url"]}
        self.write_journal([ordinary])
        self.record_atcoder_claude()
        normal = records.encode(self.atcoder_row())
        cases = []
        for field in ["evidence_kind", "needs_source_confirmation", "needs_resubmission",
                      "expected_source_sha256"]:
            changed = copy.deepcopy(record)
            del changed[field]
            cases.append(changed)
        for changes in [{"evidence_kind": "other"}, {"needs_source_confirmation": 1},
                        {"needs_resubmission": 0}, {"needs_resubmission": True},
                        {"expected_source_sha256": "invalid"},
                        {"url": "https://judge.yosupo.jp/submission/123", "history": [ordinary]},
                        {"status": "WA", "history": [record]}]:
            cases.append(dict(record, **changes))
        cases.append(dict(ordinary, history=[{"record": record}]))
        for changed in cases:
            with self.subTest(record=changed):
                self.write_journal([changed])
                self.assertEqual(self.atcoder_row()["next_stage"], "resubmit")
                self.assertEqual(records.encode(self.atcoder_row()), normal)

    def test_screenshot_confirmation_rejects_changed_and_unsafe_normalization(self):
        record = self.screenshot_record()
        self.write_journal([record])
        driver = self.root / self.entries[2]["driver"]
        original = driver.read_text()
        driver.write_text(original.replace("return 0", "return 1"))
        self.record_atcoder_claude()
        self.assertEqual(self.atcoder_row()["next_stage"], "resubmit")
        self.assertTrue(self.atcoder_row()["submission_required"])
        for text in ['auto raw = R"x(value)x";\n', '/* block comment */\n', '#define VALUE \\\n1\n']:
            with self.subTest(text=text):
                driver.write_text(original + text)
                current = self.item("unverified")["source"]
                changed = dict(record, expected_source_sha256="0" * 64,
                               expected_normalized_source_sha256=records.digest(
                                   records.status.normalized_source(current)))
                self.write_journal([changed])
                self.record_atcoder_claude()
                self.assertEqual(self.atcoder_row()["next_stage"], "resubmit")
                self.assertNotIn("source_confirmation_required", self.atcoder_row())
                # Exact bytes still qualify without normalizing special C++ text.
                changed["expected_source_sha256"] = records.digest(current)
                self.write_journal([changed])
                self.assertEqual(self.atcoder_row()["next_stage"], "confirm_source")
                self.assertFalse(self.atcoder_row()["current_ac"])

    def test_normalization_needs_uploaded_proof(self):
        source = self.item()["source"]
        record = self.ac_record()
        record["submission_sha256"] = "0" * 64
        record["normalized_source_sha256"] = records.digest(records.status.normalized_source(source))
        record["source_match"] = "Submission metadata normalization only"
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])
        record["uploaded_source_sha256"] = "1" * 64
        self.write_journal([record])
        self.assertTrue(self.item()["current_ac"])
        record["source_match"] = "Submission normalization removes all comments only"
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])

    def test_v2_expected_metadata_keeps_screenshot_in_confirmation_only(self):
        driver = self.root / self.entries[2]["driver"]
        driver.write_text(driver.read_text() + "/* a body marker */\n")
        source = self.item("unverified")["source"]
        record = self.screenshot_record()
        record.update(expected_source_sha256="0" * 64,
                      expected_source_normalization=records.status.PREAMBLE_NORMALIZATION,
                      expected_normalized_source_sha256=records.digest(
                          records.status.normalized_preamble_source(source)))
        annotated = source.replace(b"// Verify: ", b"// Submission: https://atcoder.jp/contests/abc001/submissions/123\n// Verify: ", 1)
        self.assertTrue(records.pending_source_confirmation(record, "atcoder.jp", annotated))
        self.assertIsNone(records.status.source_match(record, annotated))
        self.assertFalse(records.pending_source_confirmation(record, "atcoder.jp",
                         annotated.replace(b"a body marker", b"changed body marker")))

    def test_special_cpp_is_not_normalized(self):
        path = self.root / self.entries[0]["driver"]
        path.write_text(path.read_text() + 'auto raw = R"x(// Submission: not metadata)x";\n')
        source = self.item()["source"]
        record = self.ac_record()
        record["submission_sha256"] = "0" * 64
        record["uploaded_source_sha256"] = "1" * 64
        record["normalized_source_sha256"] = records.digest(records.status.normalized_source(source))
        record["source_normalization"] = "submission-metadata-v1"
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])

    def test_library_driver_and_io_changes_require_resubmission(self):
        for relative in ["DataStructure/a.cpp", self.entries[0]["driver"], "Verify/current/common/fast_io.hpp"]:
            path = self.root / relative
            old = path.read_bytes()
            path.write_bytes(old + b"// changed\n")
            row = records.queue(self.root, self.ledger)["entries"][0]
            self.assertFalse(row["current_ac"])
            self.assertTrue(row["submission_required"])
            path.write_bytes(old)

    def test_stale_receipt_and_tampered_evidence_are_rejected(self):
        receipt = self.receipt_file(self.receipt())
        self.proof.write_text("changed")
        with self.assertRaisesRegex(ValueError, "根拠"):
            records.record_stage(self.root, self.ledger, receipt)
        self.proof.write_text("measured")
        receipt = self.receipt_file(self.receipt())
        self.root.joinpath("DataStructure/a.cpp").write_text("int different;\n")
        with self.assertRaisesRegex(ValueError, "提出コード"):
            records.record_stage(self.root, self.ledger, receipt)

    def test_unmeasured_and_inconsistent_receipts_are_rejected(self):
        for mutate in [lambda r: r.update(changed=True),
                       lambda r: r.update(checks={"correctness": "passed", "performance": "not_run"}),
                       lambda r: r.update(evidence=[]),
                       lambda r: r.update(format=True)]:
            receipt = self.receipt()
            mutate(receipt)
            with self.assertRaises(ValueError):
                records.validate_receipt(self.root, receipt)

    def test_invalid_ids_are_rejected(self):
        for name in ["../escape", "/absolute", "bad\nID", "bad\\ID", "", "a.b"]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                records.safe_id(name)

    def test_symlinked_library_cannot_escape_repository(self):
        path = self.root / "DataStructure/a.cpp"
        path.unlink()
        outside = self.base / "outside.cpp"
        outside.write_text("private")
        path.symlink_to(outside)
        with self.assertRaises(ValueError):
            records.catalog(self.root)

    def test_record_no_change_completes_only_with_current_ac(self):
        codex = self.receipt_file(self.receipt(), "codex.json")
        records.record_stage(self.root, self.ledger, codex)
        self.assertEqual(records.queue(self.root, self.ledger)["entries"][0]["next_stage"], "claude")
        claude = self.receipt_file(self.receipt("claude"), "claude.json")
        records.record_stage(self.root, self.ledger, claude)
        self.assertEqual(records.queue(self.root, self.ledger)["entries"][0]["next_stage"], "complete")

    def test_changed_claude_result_requires_new_ac_and_keeps_external_codex(self):
        codex_value = self.receipt()
        codex = self.receipt_file(codex_value, "codex.json")
        path = self.root / "DataStructure/a.cpp"
        path.write_bytes(path.read_bytes() + b"// optimized\n")
        claude = self.receipt_file(self.receipt("claude", before=codex_value["source_sha256"]), "claude.json")
        with self.assertRaisesRegex(ValueError, "先行Codex"):
            records.record_stage(self.root, self.ledger, claude)
        records.record_stage(self.root, self.ledger, claude, codex_receipt=codex)
        row = records.queue(self.root, self.ledger)["entries"][0]
        self.assertEqual(row["next_stage"], "resubmit")
        self.assertTrue(row["submission_required"])
        self.write_journal([self.ac_record()])
        self.assertEqual(records.queue(self.root, self.ledger)["entries"][0]["next_stage"], "complete")

    def test_ledger_receipts_are_immutable_and_tampering_is_detected(self):
        receipt = self.receipt_file(self.receipt())
        path = records.record_stage(self.root, self.ledger, receipt)
        self.assertEqual(records.record_stage(self.root, self.ledger, receipt), path)
        changed = json.loads(path.read_text())
        changed["scope"] = "library"
        path.write_bytes(records.encode(changed))
        with self.assertRaisesRegex(ValueError, "名前と内容"):
            records.queue(self.root, self.ledger)

    def test_proven_submission_metadata_alias_keeps_completed_stages(self):
        before = self.item()["source"]
        codex = self.receipt_file(self.receipt(), "codex.json")
        records.record_stage(self.root, self.ledger, codex)
        claude = self.receipt_file(self.receipt("claude"), "claude.json")
        records.record_stage(self.root, self.ledger, claude)
        self.entries[0]["submission_urls"] = ["https://judge.yosupo.jp/submission/124"]
        self.group.write_text(json.dumps({"entries": self.entries}))
        current = self.item()["source"]
        self.assertNotEqual(records.digest(before), records.digest(current))
        record = {"id": "accepted", "status": "AC", "url": "https://judge.yosupo.jp/submission/124",
                  "submission_sha256": records.digest(before), "uploaded_source_sha256": records.digest(before),
                  "normalized_source_sha256": records.digest(records.status.normalized_source(before)),
                  "source_normalization": "submission-metadata-v1"}
        self.write_journal([record])
        row = records.queue(self.root, self.ledger)["entries"][0]
        self.assertTrue(row["current_ac"])
        self.assertEqual(row["next_stage"], "complete")
        # A changed main must invalidate the alias even if current_* claims are updated.
        path = self.root / self.entries[0]["driver"]
        path.write_text(path.read_text().replace("return 0", "return 1"))
        record["current_submission_sha256"] = self.item()["source_sha256"]
        record["matched_current_generation"] = True
        self.write_journal([record])
        row = records.queue(self.root, self.ledger)["entries"][0]
        self.assertFalse(row["current_ac"])
        self.assertEqual(row["next_stage"], "codex")

    def test_nested_common_and_acl_inputs_are_hashed(self):
        common = self.root / "Verify/current/common/fast_io.hpp"
        common.write_text('#include "Verify/current/common/helper.hpp"\n')
        (common.parent / "helper.hpp").write_text("int helper_value;\n")
        vendor = self.root / "Verify/current/vendor/atcoder"
        vendor.mkdir(parents=True)
        (vendor / "modint.hpp").write_text('#include "atcoder/internal_math"\nint mint_stub;\n')
        (vendor / "internal_math.hpp").write_text("int math_stub;\n")
        driver = self.root / self.entries[0]["driver"]
        driver.write_text(driver.read_text() + "#include <atcoder/modint>\n")
        hashes = self.item()["dependency_hashes"]
        for name in ["Verify/current/common/helper.hpp", "Verify/current/vendor/atcoder/modint.hpp",
                     "Verify/current/vendor/atcoder/internal_math.hpp"]:
            self.assertEqual(hashes[name], records.digest((self.root / name).read_bytes()))

    def test_symlinked_common_header_cannot_escape_repository(self):
        path = self.root / "Verify/current/common/fast_io.hpp"
        path.unlink()
        outside = self.base / "outside.hpp"
        outside.write_text("private")
        path.symlink_to(outside)
        with self.assertRaises(ValueError):
            records.catalog(self.root)

    def test_problem_url_must_be_trusted_https(self):
        self.entries[0]["problem_url"] = "http://judge.yosupo.jp/problem/unionfind"
        self.group.write_text(json.dumps({"entries": self.entries}))
        with self.assertRaises(ValueError):
            records.catalog(self.root)

    def test_library_only_receipt_is_not_library_checker_stage(self):
        value = self.receipt()
        value["scope"] = "library"
        path = self.receipt_file(value)
        with self.assertRaises(ValueError):
            records.record_stage(self.root, self.ledger, path)

    def test_non_library_checker_new_records_require_library_scope(self):
        for url in ["https://atcoder.jp/contests/abc001/tasks/abc001_1",
                    "https://yukicoder.me/problems/no/430",
                    "https://onlinejudge.u-aizu.ac.jp/problems/GRL_1_B"]:
            with self.subTest(url=url):
                self.entries[2]["problem_url"] = url
                self.group.write_text(json.dumps({"entries": self.entries}))
                value = self.receipt(identity="unverified")
                value["scope"] = "whole_program"
                with self.assertRaisesRegex(ValueError, "本体だけ"):
                    records.record_stage(self.root, self.ledger, self.receipt_file(value))
                value["scope"] = "library"
                self.assertTrue(records.record_stage(
                    self.root, self.ledger, self.receipt_file(value)).is_file())

    def test_old_whole_program_records_remain_readable(self):
        value = self.receipt(identity="unverified")
        value["scope"] = "whole_program"
        self.ledger.mkdir(parents=True)
        path = self.ledger / f"unverified.codex.{records.digest(records.encode(value))}.json"
        path.write_bytes(records.encode(value))
        self.assertEqual(records.ledger(self.root, self.ledger), [value])

    def test_catalog_rejects_dependency_edit_during_render(self):
        original = records.verify.render
        def changed(*args, **kwargs):
            result = original(*args, **kwargs)
            path = self.root / "Verify/current/common/fast_io.hpp"
            path.write_bytes(path.read_bytes() + b"// concurrent edit\n")
            return result
        with patch.object(records.verify, "render", side_effect=changed):
            with self.assertRaisesRegex(ValueError, "生成中"):
                records.catalog(self.root)

    def test_cli_list_is_readonly(self):
        before = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob("*") if p.is_file()}
        command = [sys.executable, str(Path(records.__file__)), "--root", str(self.root), "--list"]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(json.loads(result.stdout)["entries"]), 1)
        after = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob("*") if p.is_file()}
        self.assertEqual(before, after)


if __name__ == "__main__":
    unittest.main()
