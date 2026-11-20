#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""最適化の完了記録と、提出コード・測定根拠の対応を確認する。"""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
from urllib.parse import urlsplit

try:
    from . import prepare_verify as verify
    from . import update_verify_status as status
except ImportError:
    import prepare_verify as verify
    import update_verify_status as status

ROOT = Path(__file__).resolve().parents[1]
SHA = re.compile(r"[0-9a-f]{64}\Z")
IDENTIFIER = re.compile(r"[A-Za-z0-9][A-Za-z0-9_-]*\Z")
AUDIT_RECORD_DIRECTORIES = ("Changes", "Distribution", "Performance", "Provenance",
                            "Submissions", "Verification")


def encode(value: object) -> bytes:
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2) + "\n").encode()


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def safe_id(value: str) -> str:
    if not isinstance(value, str) or not IDENTIFIER.fullmatch(value):
        raise ValueError("不正なVerify ID")
    return value


def source_path(root: Path, name: str) -> Path:
    p = PurePosixPath(name)
    if not name or p.is_absolute() or any(part in {".", ".."} for part in p.parts) or "\\" in name:
        raise ValueError("不正なソース相対パス: " + name)
    target = (root / name).resolve()
    if not target.is_relative_to(root.resolve()) or not target.is_file():
        raise ValueError("リポジトリ内のファイルではありません: " + name)
    return target


@contextmanager
def verifier_root(root: Path):
    previous = verify.ROOT, verify.BASE
    verify.ROOT, verify.BASE = root, root / "Verify/current"
    try:
        yield
    finally:
        verify.ROOT, verify.BASE = previous


def accepted(record: dict, host: str) -> bool:
    url = record.get("url")
    return (record.get("status") == "AC" and isinstance(url, str)
            and verify.ONLINE_JUDGE_NAMES.get(host) is not None
            and verify.ONLINE_JUDGE_NAMES.get(urlsplit(url).hostname)
                == verify.ONLINE_JUDGE_NAMES.get(host))


def pending_source_confirmation(record: dict | None, host: str, source: bytes) -> bool:
    """期待本文は確認待ちの判定だけに使い、オンラインACの証拠にしない。"""
    if (record is None or not accepted(record, host)
            or record.get("evidence_kind") != "user_screenshot"
            or record.get("needs_source_confirmation") is not True
            or record.get("needs_resubmission") is not False):
        return False
    expected = record.get("expected_source_sha256")
    if not isinstance(expected, str) or not SHA.fullmatch(expected):
        return False
    if expected == digest(source):
        return True
    normalized = record.get("expected_normalized_source_sha256")
    return status.normalization_matches(record.get("expected_source_normalization"), normalized, source)


def dependency_hashes(root: Path, texts: list[str]) -> dict[str, str]:
    result = {}
    pending = list(texts)
    acl_include = re.compile(r'^#include [<"](atcoder/[a-z_]+)(?:\.hpp)?[>"]\s*$', re.M)
    while pending:
        text = pending.pop()
        names = list(verify.COMMON_INCLUDE.findall(text))
        names.extend("Verify/current/vendor/" + name.removesuffix(".hpp") + ".hpp"
                     for name in acl_include.findall(text))
        for name in names:
            if name in result:
                continue
            data = source_path(root, name).read_bytes()
            result[name] = digest(data)
            pending.append(data.decode())
    return result


def catalog(root: Path) -> list[dict]:
    root = root.resolve()
    journal = root / "Verify/current/judgements.json"
    items = json.loads(journal.read_text())["entries"] if journal.exists() else []
    latest = {item["id"]: item for item in status.records_from(items)}
    records = {identity: status.history(item) for identity, item in latest.items()}
    result = []
    with verifier_root(root):
        catalog_entries = verify.collect()
        policies = status.submission_policy(root, catalog_entries)
        for entry in catalog_entries:
            identity = safe_id(entry["id"])
            for name in [*entry["libraries"], entry["driver"]]:
                source_path(root, name)
            library_data = {name: source_path(root, name).read_bytes() for name in entry["libraries"]}
            driver = source_path(root, entry["driver"]).read_bytes()
            dependencies = dependency_hashes(root, [driver.decode(), *(data.decode() for data in library_data.values())])
            if entry.get("problem_url"):
                status.safe_url(entry["problem_url"])
            source, library_hashes = verify.render(entry, source_overrides=library_data,
                                                    driver_override=driver.decode())
            if any(digest(source_path(root, name).read_bytes()) != expected for name, expected in dependencies.items()):
                raise ValueError("提出コードの生成中に依存ファイルが変更されました")
            common = {name: sha for name, sha in dependencies.items() if name.startswith("Verify/current/common/")}
            host = urlsplit(entry.get("problem_url") or "").hostname
            ac = [r for r in records.get(identity, []) if accepted(r, host)]
            current_matches = [r for r in ac if status.source_match(r, source) is not None]
            aliases = {digest(source)}
            for record in current_matches:
                aliases.update(record[field] for field in ("submission_sha256", "uploaded_source_sha256")
                               if isinstance(record.get(field), str) and SHA.fullmatch(record[field]))
            item = {"id": identity, "entry": entry, "source": source,
                           "source_sha256": digest(source), "library_hashes": library_hashes,
                           "driver_sha256": digest(driver), "common_hashes": common,
                           "dependency_hashes": dependencies,
                           "library_checker": host == "judge.yosupo.jp",
                           "historical_ac": bool(ac),
                           "current_ac": bool(current_matches), "accepted_source_aliases": sorted(aliases)}
            action = status.submission_action(policies.get(identity), digest(source))
            if action:
                item["submission_policy"] = action
            if pending_source_confirmation(latest.get(identity), host, source):
                # stageの本文照合専用。受理済み本文の別名には加えない。
                item["source_confirmation_expected_sha256"] = latest[identity]["expected_source_sha256"]
            result.append(item)
    return sorted(result, key=lambda item: item["id"])


def evidence_path(root: Path, name: str) -> Path:
    if not isinstance(name, str) or not name or "\x00" in name:
        raise ValueError("不正な根拠ファイルのパス")
    p = Path(name)
    if not p.is_absolute():
        parts = PurePosixPath(name).parts
        if (len(parts) == 2 and parts[0] == "Audit" and name == "/".join(parts)
                and not (root / name).exists() and not (root / name).is_symlink()):
            # 内容ハッシュ付きの旧記録は変更せず、移動した根拠の同名ファイルを探す。
            matches = [f"Audit/{directory}/{parts[1]}" for directory in AUDIT_RECORD_DIRECTORIES
                       if (root / "Audit" / directory / parts[1]).is_file()]
            if len(matches) > 1:
                raise ValueError("旧パスに対応する根拠ファイルが複数あります: " + name)
            if matches:
                return source_path(root, matches[0])
        p = source_path(root, name)
    if not p.is_file():
        raise ValueError("根拠ファイルが存在しません: " + name)
    return p


def validate_receipt(root: Path, value: dict, *, identity: str | None = None,
                     stage: str | None = None, source_sha256: str | None = None) -> dict:
    if not isinstance(value, dict) or type(value.get("format")) is not int or value.get("format") != 1:
        raise ValueError("stage receiptのformatは1が必要です")
    safe_id(value.get("id"))
    if value.get("stage") not in {"codex", "claude"} or value.get("status") != "complete":
        raise ValueError("完了したcodex/claude stageが必要です")
    if identity is not None and value["id"] != identity:
        raise ValueError("receiptのVerify IDが一致しません")
    if stage is not None and value["stage"] != stage:
        raise ValueError("receiptの担当が一致しません")
    for field in ["input_source_sha256", "source_sha256"]:
        if not isinstance(value.get(field), str) or not SHA.fullmatch(value[field]):
            raise ValueError("receiptの" + field + "が不正です")
    if source_sha256 is not None and value["source_sha256"] != source_sha256:
        raise ValueError("receiptと現在の提出コードが一致しません")
    if type(value.get("changed")) is not bool or value["changed"] != (value["input_source_sha256"] != value["source_sha256"]):
        raise ValueError("changedと入出力のソースSHAが一致しません")
    if value.get("scope") not in {"whole_program", "library"}:
        raise ValueError("scopeはwhole_programまたはlibraryが必要です")
    if value.get("checks") != {"correctness": "passed", "performance": "measured"}:
        raise ValueError("正しさと性能の実測確認が必要です")
    proofs = value.get("evidence")
    if not isinstance(proofs, list) or not proofs:
        raise ValueError("測定・検証の根拠ファイルが必要です")
    for proof in proofs:
        if not isinstance(proof, dict) or not isinstance(proof.get("sha256"), str) or not SHA.fullmatch(proof["sha256"]):
            raise ValueError("不正な根拠ファイルのSHA")
        if digest(evidence_path(root, proof.get("path")).read_bytes()) != proof["sha256"]:
            raise ValueError("根拠ファイルの内容が変わっています")
    return value


def read_receipt(root: Path, path: Path, **requirements) -> dict:
    return validate_receipt(root, json.loads(path.read_text()), **requirements)


def ledger(root: Path, directory: Path) -> list[dict]:
    rows = []
    for path in sorted(directory.glob("*.json")):
        value = read_receipt(root, path)
        expected = f"{value['id']}.{value['stage']}.{digest(encode(value))}.json"
        if path.name != expected:
            raise ValueError("stage記録の名前と内容が一致しません: " + path.name)
        rows.append(value)
    return rows


def codex_predecessor(item: dict, receipts: list[dict], input_sha: str) -> bool:
    return any(r["id"] == item["id"] and r["stage"] == "codex"
               and r["source_sha256"] == input_sha and r["scope"] == "whole_program"
               for r in receipts)


def queue(root: Path, ledger_dir: Path, include_unverified: bool = False) -> dict:
    entries = catalog(root)
    receipts = ledger(root, ledger_dir)
    rows = []
    for item in entries:
        entry = item["entry"]
        if not entry.get("problem_url") or not (item["historical_ac"] or include_unverified):
            continue
        action = item.get("submission_policy", {})
        if action.get("kind") == "reference":
            continue
        if action.get("kind") == "documentation_only":
            rows.append({"id": item["id"], "problem_url": entry["problem_url"],
                         "libraries": entry["libraries"], "source_sha256": item["source_sha256"],
                         "current_ac": item["current_ac"], "historical_ac": item["historical_ac"],
                         "submission_required": False, "next_stage": "exempt",
                         "scope": "whole_program" if item["library_checker"] else "library",
                         "reason": action["reason"]})
            continue
        stage_aliases = set(item["accepted_source_aliases"])
        confirmation = item.get("source_confirmation_expected_sha256")
        if confirmation is not None:
            stage_aliases.add(confirmation)
        matching = [r for r in receipts if r["id"] == item["id"] and r["source_sha256"] in stage_aliases]
        claude_done = any(r["stage"] == "claude" and (not item["library_checker"]
                          or codex_predecessor(item, receipts, r["input_source_sha256"])) for r in matching)
        codex_done = any(r["stage"] == "codex" and r["scope"] == "whole_program" for r in matching)
        next_stage = ("complete" if item["current_ac"] else "resubmit") if claude_done else (
            "claude" if codex_done or not item["library_checker"] else "codex")
        row = {"id": item["id"], "problem_url": entry["problem_url"],
                     "libraries": entry["libraries"], "source_sha256": item["source_sha256"],
                     "current_ac": item["current_ac"], "historical_ac": item["historical_ac"],
                     "submission_required": not item["current_ac"], "next_stage": next_stage,
                     "scope": "whole_program" if item["library_checker"] else "library"}
        if claude_done and not item["current_ac"] and confirmation is not None:
            row.update(next_stage="confirm_source", submission_required=False,
                       source_confirmation_required=True)
        rows.append(row)
    return {"format": 1, "entries": rows}


def record_stage(root: Path, ledger_dir: Path, receipt_path: Path,
                 codex_receipt: Path | None = None) -> Path:
    value = read_receipt(root, receipt_path)
    entry = next((e for e in catalog(root) if e["id"] == value["id"]), None)
    if entry is None:
        raise ValueError("存在しないVerify ID")
    validate_receipt(root, value, source_sha256=entry["source_sha256"])
    external_predecessor = None
    if entry["library_checker"]:
        if value["scope"] != "whole_program":
            raise ValueError("Library Checkerではmain・入出力を含む全体が対象です")
        if value["stage"] == "claude":
            predecessors = ledger(root, ledger_dir)
            if codex_receipt is not None:
                external_predecessor = read_receipt(root, codex_receipt, identity=value["id"], stage="codex")
                predecessors.append(external_predecessor)
            if not codex_predecessor(entry, predecessors, value["input_source_sha256"]):
                raise ValueError("Claude入力に一致する先行Codex receiptが必要です")
    elif value["scope"] != "library":
        raise ValueError("Library Checker以外ではライブラリ本体だけが最適化対象です")
    ledger_dir.mkdir(parents=True, exist_ok=True)
    values = [value]
    if external_predecessor is not None and codex_predecessor(entry, [external_predecessor], value["input_source_sha256"]):
        values.insert(0, external_predecessor)
    for receipt in values:
        target = ledger_dir / f"{receipt['id']}.{receipt['stage']}.{digest(encode(receipt))}.json"
        if target.exists():
            if target.read_bytes() != encode(receipt):
                raise ValueError("既存のstage記録は上書きできません")
        else:
            with target.open("xb") as handle:
                handle.write(encode(receipt))
    return target



def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    command = parser.add_mutually_exclusive_group(required=True)
    command.add_argument("--list", action="store_true")
    command.add_argument("--record-stage", type=Path, metavar="JSON")
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--ledger-dir", type=Path)
    parser.add_argument("--codex-receipt", type=Path)
    parser.add_argument("--include-unverified", action="store_true", help="一覧に未受理の問題も含める")
    options = parser.parse_args()
    root = options.root.resolve()
    ledger_dir = options.ledger_dir or root / "Verify/optimization/stages"
    try:
        if options.list:
            print(encode(queue(root, ledger_dir, options.include_unverified)).decode(), end="")
        else:
            print(record_stage(root, ledger_dir, options.record_stage, options.codex_receipt))
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
