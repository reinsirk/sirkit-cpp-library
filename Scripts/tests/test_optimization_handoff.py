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
import optimization_handoff as handoff


class OptimizationHandoffTest(unittest.TestCase):
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
        return next(e for e in handoff.catalog(self.root) if e["id"] == name)

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
                "needs_resubmission": False, "expected_source_sha256": handoff.digest(source),
                "expected_normalized_source_sha256": handoff.digest(handoff.status.normalized_source(source)),
                "expected_source_normalization": "submission-metadata-v1"}

    def record_atcoder_claude(self):
        value = self.receipt("claude", identity="unverified")
        handoff.record_stage(self.root, self.ledger, self.receipt_file(value))

    def atcoder_row(self):
        return next(row for row in handoff.queue(self.root, self.ledger)["entries"]
                    if row["id"] == "unverified")

    def receipt(self, stage="codex", *, before=None, identity="accepted"):
        after = self.item(identity)["source_sha256"]
        return {"format": 1, "id": identity, "stage": stage, "status": "complete",
                "input_source_sha256": before or after, "source_sha256": after,
                "changed": bool(before and before != after), "scope": "whole_program",
                "checks": {"correctness": "passed", "performance": "measured"},
                "evidence": [{"path": "Audit/proof.json", "sha256": handoff.digest(self.proof.read_bytes())}]}

    def receipt_file(self, value, name="receipt.json"):
        path = self.base / name
        path.write_bytes(handoff.encode(value))
        return path

    def test_default_queue_only_has_accepted_history(self):
        rows = handoff.queue(self.root, self.ledger)["entries"]
        self.assertEqual([r["id"] for r in rows], ["accepted"])
        self.assertTrue(rows[0]["current_ac"])
        self.assertEqual(rows[0]["next_stage"], "codex")
        self.assertEqual(len(handoff.queue(self.root, self.ledger, True)["entries"]), 3)

    def test_no_journal_and_corrupt_journal(self):
        self.journal.unlink()
        self.assertEqual(handoff.queue(self.root, self.ledger)["entries"], [])
        self.write_journal({"not": "an array"})
        with self.assertRaises(ValueError):
            handoff.catalog(self.root)

    def test_duplicate_record_id_is_rejected(self):
        row = self.ac_record()
        self.write_journal([row, row])
        with self.assertRaises(ValueError):
            handoff.catalog(self.root)

    def test_orphan_record_is_not_queued(self):
        self.write_journal([self.ac_record(), {"id": "orphan", "status": "AC", "url": "https://judge.yosupo.jp/submission/1"}])
        self.assertEqual(len(handoff.queue(self.root, self.ledger)["entries"]), 1)

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
        state = handoff.status.build_status(self.root)
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
        normal = handoff.encode(self.atcoder_row())
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
                self.assertEqual(handoff.encode(self.atcoder_row()), normal)

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
                               expected_normalized_source_sha256=handoff.digest(
                                   handoff.status.normalized_source(current)))
                self.write_journal([changed])
                self.record_atcoder_claude()
                self.assertEqual(self.atcoder_row()["next_stage"], "resubmit")
                self.assertNotIn("source_confirmation_required", self.atcoder_row())
                # Exact bytes still qualify without normalizing special C++ text.
                changed["expected_source_sha256"] = handoff.digest(current)
                self.write_journal([changed])
                self.assertEqual(self.atcoder_row()["next_stage"], "confirm_source")
                self.assertFalse(self.atcoder_row()["current_ac"])

    def test_normalization_needs_uploaded_proof(self):
        source = self.item()["source"]
        record = self.ac_record()
        record["submission_sha256"] = "0" * 64
        record["normalized_source_sha256"] = handoff.digest(handoff.status.normalized_source(source))
        record["source_match"] = "Submission metadata normalization only"
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])
        record["uploaded_source_sha256"] = "1" * 64
        self.write_journal([record])
        self.assertTrue(self.item()["current_ac"])
        record["source_match"] = "Submission normalization removes all comments only"
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])

    def test_special_cpp_is_not_normalized(self):
        path = self.root / self.entries[0]["driver"]
        path.write_text(path.read_text() + 'auto raw = R"x(// Submission: not metadata)x";\n')
        source = self.item()["source"]
        record = self.ac_record()
        record["submission_sha256"] = "0" * 64
        record["uploaded_source_sha256"] = "1" * 64
        record["normalized_source_sha256"] = handoff.digest(handoff.status.normalized_source(source))
        record["source_normalization"] = "submission-metadata-v1"
        self.write_journal([record])
        self.assertFalse(self.item()["current_ac"])

    def test_library_driver_and_io_changes_require_resubmission(self):
        for relative in ["DataStructure/a.cpp", self.entries[0]["driver"], "Verify/current/common/fast_io.hpp"]:
            path = self.root / relative
            old = path.read_bytes()
            path.write_bytes(old + b"// changed\n")
            row = handoff.queue(self.root, self.ledger)["entries"][0]
            self.assertFalse(row["current_ac"])
            self.assertTrue(row["submission_required"])
            path.write_bytes(old)

    def test_codex_bundle_is_frozen_with_dependencies_and_fanout(self):
        destination = self.base / "bundle"
        manifest = handoff.prepare(self.root, "accepted", destination, stage="codex")
        self.assertEqual(handoff.check_bundle(destination), manifest)
        self.assertEqual(manifest["affected_library_verify_ids"], ["accepted", "shared"])
        self.assertIn("inputs/AGENTS.md", manifest["files"])
        self.assertIn("inputs/Verify/current/common/fast_io.hpp", manifest["files"])
        self.assertIn("prompt.md", manifest["files"])
        self.assertEqual((destination / "submission.cpp").read_bytes(), self.item()["source"])

    def test_prepare_claude_requires_codex_receipt(self):
        with self.assertRaisesRegex(ValueError, "codex-receipt"):
            handoff.prepare(self.root, "accepted", self.base / "bundle")
        receipt = self.receipt_file(self.receipt())
        handoff.prepare(self.root, "accepted", self.base / "bundle", codex_receipt=receipt)
        self.assertTrue((self.base / "bundle/codex_receipt.json").exists())

    def test_non_library_checker_can_prepare_claude_directly(self):
        handoff.prepare(self.root, "unverified", self.base / "bundle")
        self.assertFalse((self.base / "bundle/codex_receipt.json").exists())

    def test_stale_receipt_and_tampered_evidence_are_rejected(self):
        receipt = self.receipt_file(self.receipt())
        self.proof.write_text("changed")
        with self.assertRaisesRegex(ValueError, "根拠"):
            handoff.prepare(self.root, "accepted", self.base / "bundle", codex_receipt=receipt)
        self.proof.write_text("measured")
        receipt = self.receipt_file(self.receipt())
        self.root.joinpath("DataStructure/a.cpp").write_text("int different;\n")
        with self.assertRaisesRegex(ValueError, "提出コード"):
            handoff.prepare(self.root, "accepted", self.base / "bundle", codex_receipt=receipt)

    def test_unmeasured_and_inconsistent_receipts_are_rejected(self):
        for mutate in [lambda r: r.update(changed=True),
                       lambda r: r.update(checks={"correctness": "passed", "performance": "not_run"}),
                       lambda r: r.update(evidence=[]),
                       lambda r: r.update(format=True)]:
            receipt = self.receipt()
            mutate(receipt)
            with self.assertRaises(ValueError):
                handoff.validate_receipt(self.root, receipt)

    def test_safe_ids_and_repository_output_guard(self):
        for name in ["../escape", "/absolute", "bad\nID", "bad\\ID", "", "a.b"]:
            with self.assertRaises(ValueError):
                handoff.prepare(self.root, name, self.base / "bundle", stage="codex")
        with self.assertRaises(ValueError):
            handoff.prepare(self.root, "accepted", self.root / "bundle", stage="codex")
        with self.assertRaises(ValueError):
            handoff.prepare(self.root, "accepted", self.root.parent, stage="codex")

    def test_symlinked_library_cannot_escape_repository(self):
        path = self.root / "DataStructure/a.cpp"
        path.unlink()
        outside = self.base / "outside.cpp"
        outside.write_text("private")
        path.symlink_to(outside)
        with self.assertRaises(ValueError):
            handoff.catalog(self.root)

    def test_existing_bundle_is_not_overwritten(self):
        destination = self.base / "bundle"
        destination.mkdir()
        keep = destination / "keep"
        keep.write_text("keep")
        with self.assertRaises(ValueError):
            handoff.prepare(self.root, "accepted", destination, stage="codex")
        self.assertEqual(keep.read_text(), "keep")

    def test_check_bundle_detects_source_prompt_and_listing_tamper(self):
        destination = self.base / "bundle"
        handoff.prepare(self.root, "accepted", destination, stage="codex")
        for name in ["submission.cpp", "prompt.md"]:
            path = destination / name
            old = path.read_bytes()
            path.write_bytes(old + b"changed")
            with self.assertRaises(ValueError):
                handoff.check_bundle(destination)
            path.write_bytes(old)
        (destination / "extra.txt").write_text("unhashed")
        with self.assertRaises(ValueError):
            handoff.check_bundle(destination)

    def test_record_no_change_completes_only_with_current_ac(self):
        codex = self.receipt_file(self.receipt(), "codex.json")
        handoff.record_stage(self.root, self.ledger, codex)
        self.assertEqual(handoff.queue(self.root, self.ledger)["entries"][0]["next_stage"], "claude")
        claude = self.receipt_file(self.receipt("claude"), "claude.json")
        handoff.record_stage(self.root, self.ledger, claude)
        self.assertEqual(handoff.queue(self.root, self.ledger)["entries"][0]["next_stage"], "complete")

    def test_changed_claude_result_requires_new_ac_and_keeps_external_codex(self):
        codex_value = self.receipt()
        codex = self.receipt_file(codex_value, "codex.json")
        path = self.root / "DataStructure/a.cpp"
        path.write_bytes(path.read_bytes() + b"// optimized\n")
        claude = self.receipt_file(self.receipt("claude", before=codex_value["source_sha256"]), "claude.json")
        with self.assertRaisesRegex(ValueError, "先行Codex"):
            handoff.record_stage(self.root, self.ledger, claude)
        handoff.record_stage(self.root, self.ledger, claude, codex_receipt=codex)
        row = handoff.queue(self.root, self.ledger)["entries"][0]
        self.assertEqual(row["next_stage"], "resubmit")
        self.assertTrue(row["submission_required"])
        self.write_journal([self.ac_record()])
        self.assertEqual(handoff.queue(self.root, self.ledger)["entries"][0]["next_stage"], "complete")

    def test_ledger_receipts_are_immutable_and_tampering_is_detected(self):
        receipt = self.receipt_file(self.receipt())
        path = handoff.record_stage(self.root, self.ledger, receipt)
        self.assertEqual(handoff.record_stage(self.root, self.ledger, receipt), path)
        changed = json.loads(path.read_text())
        changed["scope"] = "library"
        path.write_bytes(handoff.encode(changed))
        with self.assertRaisesRegex(ValueError, "名前と内容"):
            handoff.queue(self.root, self.ledger)

    def test_proven_submission_metadata_alias_keeps_completed_stages(self):
        before = self.item()["source"]
        codex = self.receipt_file(self.receipt(), "codex.json")
        handoff.record_stage(self.root, self.ledger, codex)
        claude = self.receipt_file(self.receipt("claude"), "claude.json")
        handoff.record_stage(self.root, self.ledger, claude)
        self.entries[0]["submission_urls"] = ["https://judge.yosupo.jp/submission/124"]
        self.group.write_text(json.dumps({"entries": self.entries}))
        current = self.item()["source"]
        self.assertNotEqual(handoff.digest(before), handoff.digest(current))
        record = {"id": "accepted", "status": "AC", "url": "https://judge.yosupo.jp/submission/124",
                  "submission_sha256": handoff.digest(before), "uploaded_source_sha256": handoff.digest(before),
                  "normalized_source_sha256": handoff.digest(handoff.status.normalized_source(before)),
                  "source_normalization": "submission-metadata-v1"}
        self.write_journal([record])
        row = handoff.queue(self.root, self.ledger)["entries"][0]
        self.assertTrue(row["current_ac"])
        self.assertEqual(row["next_stage"], "complete")
        # A changed main must invalidate the alias even if current_* claims are updated.
        path = self.root / self.entries[0]["driver"]
        path.write_text(path.read_text().replace("return 0", "return 1"))
        record["current_submission_sha256"] = self.item()["source_sha256"]
        record["matched_current_generation"] = True
        self.write_journal([record])
        row = handoff.queue(self.root, self.ledger)["entries"][0]
        self.assertFalse(row["current_ac"])
        self.assertEqual(row["next_stage"], "codex")

    def test_nested_common_and_acl_inputs_are_frozen(self):
        common = self.root / "Verify/current/common/fast_io.hpp"
        common.write_text('#include "Verify/current/common/helper.hpp"\n')
        (common.parent / "helper.hpp").write_text("int helper_value;\n")
        vendor = self.root / "Verify/current/vendor/atcoder"
        vendor.mkdir(parents=True)
        (vendor / "modint.hpp").write_text('#include "atcoder/internal_math"\nint mint_stub;\n')
        (vendor / "internal_math.hpp").write_text("int math_stub;\n")
        driver = self.root / self.entries[0]["driver"]
        driver.write_text(driver.read_text() + "#include <atcoder/modint>\n")
        manifest = handoff.prepare(self.root, "accepted", self.base / "bundle", stage="codex")
        for name in ["Verify/current/common/helper.hpp", "Verify/current/vendor/atcoder/modint.hpp",
                     "Verify/current/vendor/atcoder/internal_math.hpp"]:
            self.assertIn("inputs/" + name, manifest["files"])
        handoff.check_bundle(self.base / "bundle")

    def test_symlinked_common_header_cannot_escape_repository(self):
        path = self.root / "Verify/current/common/fast_io.hpp"
        path.unlink()
        outside = self.base / "outside.hpp"
        outside.write_text("private")
        path.symlink_to(outside)
        with self.assertRaises(ValueError):
            handoff.catalog(self.root)

    def test_problem_url_must_be_trusted_https(self):
        self.entries[0]["problem_url"] = "http://judge.yosupo.jp/problem/unionfind"
        self.group.write_text(json.dumps({"entries": self.entries}))
        with self.assertRaises(ValueError):
            handoff.catalog(self.root)

    def test_library_only_receipt_is_not_library_checker_stage(self):
        value = self.receipt()
        value["scope"] = "library"
        path = self.receipt_file(value)
        with self.assertRaises(ValueError):
            handoff.record_stage(self.root, self.ledger, path)
        with self.assertRaises(ValueError):
            handoff.prepare(self.root, "accepted", self.base / "bundle", codex_receipt=path)

    def test_bundle_rejects_inputs_changed_after_snapshot(self):
        original = handoff.catalog
        def changed(root):
            rows = original(root)
            path = self.root / "DataStructure/a.cpp"
            path.write_bytes(path.read_bytes() + b"// concurrent edit\n")
            return rows
        with patch.object(handoff, "catalog", side_effect=changed):
            with self.assertRaisesRegex(ValueError, "準備中"):
                handoff.prepare(self.root, "accepted", self.base / "bundle", stage="codex")
        self.assertFalse((self.base / "bundle").exists())

    def test_catalog_rejects_dependency_edit_during_render(self):
        original = handoff.verify.render
        def changed(*args, **kwargs):
            result = original(*args, **kwargs)
            path = self.root / "Verify/current/common/fast_io.hpp"
            path.write_bytes(path.read_bytes() + b"// concurrent edit\n")
            return result
        with patch.object(handoff.verify, "render", side_effect=changed):
            with self.assertRaisesRegex(ValueError, "生成中"):
                handoff.catalog(self.root)

    def test_cli_list_is_readonly(self):
        before = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob("*") if p.is_file()}
        command = [sys.executable, str(Path(handoff.__file__)), "--root", str(self.root), "--list"]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(json.loads(result.stdout)["entries"]), 1)
        after = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob("*") if p.is_file()}
        self.assertEqual(before, after)


if __name__ == "__main__":
    unittest.main()
