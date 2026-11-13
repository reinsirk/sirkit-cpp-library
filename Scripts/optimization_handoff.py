#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""最適化の順序・入力ソース・測定根拠を固定して、次の担当へ渡す。"""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import tempfile
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
    return record.get("status") == "AC" and isinstance(url, str) and urlsplit(url).hostname == host


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
    if (record.get("expected_source_normalization") != "submission-metadata-v1"
            or not isinstance(normalized, str) or not SHA.fullmatch(normalized)):
        return False
    # status.source_matchと同じく、特殊なC++本文では完全一致だけを使う。
    if b'/*' in source or re.search(rb'\b(?:u8|u|U|L)?R"|\\\r?\n', source):
        return False
    data = status.normalized_source(source)
    return normalized in {digest(data), digest(data + b"\n")}


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
        for entry in verify.collect():
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



def prepare(root: Path, identity: str, directory: Path, *, stage: str = "claude",
            codex_receipt: Path | None = None) -> dict:
    identity = safe_id(identity)
    if stage not in {"codex", "claude"}:
        raise ValueError("stageはcodexまたはclaudeが必要です")
    item = next((e for e in catalog(root) if e["id"] == identity), None)
    if item is None or not item["entry"].get("problem_url"):
        raise ValueError("オンラインVerifyのIDを指定してください")
    receipt = None
    if stage == "claude" and item["library_checker"]:
        if codex_receipt is None:
            raise ValueError("Library CheckerのClaude引き継ぎには--codex-receiptが必要です")
        receipt = read_receipt(root, codex_receipt, identity=identity, stage="codex",
                               source_sha256=item["source_sha256"])
        if receipt["scope"] != "whole_program":
            raise ValueError("Codexはプログラム全体の確認が必要です")
    root = root.resolve()
    directory = directory.resolve()
    if directory.is_relative_to(root) or root.is_relative_to(directory):
        raise ValueError("引き継ぎ先はリポジトリの外側へ指定してください")
    if directory.exists():
        raise ValueError("既存の引き継ぎ先は上書きしません")
    entry = item["entry"]
    dependencies = sorted(set([*entry["libraries"], entry["driver"], *item["dependency_hashes"], "AGENTS.md"]))
    data = {"submission.cpp": item["source"]}
    expected_sources = {**item["library_hashes"], entry["driver"]: item["driver_sha256"], **item["dependency_hashes"]}
    for name in dependencies:
        content = source_path(root, name).read_bytes()
        if name in expected_sources and digest(content) != expected_sources[name]:
            raise ValueError("引き継ぎ準備中にソースが変更されました: " + name)
        data["inputs/" + name] = content
    for name in ["LICENSE", "Verify/current/vendor/ACL_LICENSE"]:
        if (root / name).is_file():
            data["inputs/" + name] = source_path(root, name).read_bytes()
    if receipt is not None:
        data["codex_receipt.json"] = encode(receipt)
    files = {name: digest(value) for name, value in sorted(data.items())}
    manifest = {"format": 1, "id": identity, "stage": stage,
                "problem_url": entry["problem_url"], "source_sha256": item["source_sha256"],
                "current_ac": item["current_ac"], "historical_ac": item["historical_ac"],
                "library_hashes": item["library_hashes"], "driver_sha256": item["driver_sha256"],
                "common_hashes": item["common_hashes"], "dependency_hashes": item["dependency_hashes"], "files": files,
                "affected_library_verify_ids": sorted(e["id"] for e in catalog(root)
                    if set(entry["libraries"]).intersection(e["entry"]["libraries"]))}
    scope = "Library Checker向けなのでmain・共通入出力・ライブラリを含むプログラム全体" if item["library_checker"] else "ライブラリ本体"
    prompt = f"""# {identity}: {stage} 最適化\n\n対象: {scope}。問題: {entry['problem_url']}\n入力ソースSHA-256: {item['source_sha256']}\n\n- submission.cppが固定した現在版です。manifest.jsonの各SHAを確認してください。\n- inputs/AGENTS.mdの制約を読み、公開API・正しさ・所有権を維持してください。\n- Library CheckerはCodex→Claudeの順です。Claudeへは先行Codex receiptを添付しています。\n- 現在のオンラインAC: {item['current_ac']}。過去のACを変更後の検証済みとは扱いません。\n- 同じ問題・入力・コンパイラ設定で正しさと実行時間を測り、基準版と候補を比較してください。\n- 本体・main・入出力のどこを変えたかを区別し、本体だけの比較は同じmain/入出力で行ってください。\n- 変更はcandidate.patchとして提示し、測定条件・入力SHA・結果・不採用案も根拠ファイルへ記録してください。\n- 変更を採用したら提出コードと派生版を再生成し、ローカル照合後にオンライン再提出してください。\n- 対象外の未コミット変更・判定履歴・認証情報を変更しないでください。未実行の確認をPASSとしないでください。\n- 変更なしの結論も、実測根拠とともにstage receiptへ記録できます。\n\nstage receiptはformat=1、id、stage、status=complete、input_source_sha256、source_sha256、changed、scope、checks={{correctness:passed,performance:measured}}、evidence=[{{path,sha256}}]を持ちます。最終SHAは採用後の生成コードを指します。\n"""
    data["prompt.md"] = prompt.encode()
    manifest["files"] = {name: digest(value) for name, value in sorted(data.items())}
    data["manifest.json"] = encode(manifest)
    directory.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=".optimization-", dir=directory.parent))
    try:
        for name, content in data.items():
            path = temporary / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(content)
        temporary.rename(directory)
    finally:
        if temporary.exists():
            shutil.rmtree(temporary)
    return manifest


def check_bundle(directory: Path) -> dict:
    directory = directory.resolve()
    manifest = json.loads((directory / "manifest.json").read_text())
    if not isinstance(manifest, dict) or manifest.get("format") != 1:
        raise ValueError("不正なbundle manifest")
    safe_id(manifest.get("id"))
    files = manifest.get("files")
    if not isinstance(files, dict) or not {"submission.cpp", "prompt.md", "inputs/AGENTS.md"}.issubset(files):
        raise ValueError("bundleの必須ファイルがありません")
    actual = {p.relative_to(directory).as_posix() for p in directory.rglob("*") if p.is_file()}
    if actual != set(files) | {"manifest.json"}:
        raise ValueError("bundleのファイル一覧が一致しません")
    for name, expected in files.items():
        if not isinstance(expected, str) or not SHA.fullmatch(expected):
            raise ValueError("bundleファイルのSHAが不正です")
        if digest(source_path(directory, name).read_bytes()) != expected:
            raise ValueError("bundleファイルの内容が変わっています: " + name)
    if files["submission.cpp"] != manifest.get("source_sha256"):
        raise ValueError("bundleの提出コードSHAが一致しません")
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    command = parser.add_mutually_exclusive_group(required=True)
    command.add_argument("--list", action="store_true")
    command.add_argument("--prepare", metavar="ID")
    command.add_argument("--record-stage", type=Path, metavar="JSON")
    command.add_argument("--check-bundle", type=Path, metavar="DIRECTORY")
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--ledger-dir", type=Path)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--stage", choices=["codex", "claude"], default="claude")
    parser.add_argument("--codex-receipt", type=Path)
    parser.add_argument("--include-unverified", action="store_true", help="一覧に未受理の問題も含める")
    options = parser.parse_args()
    root = options.root.resolve()
    ledger_dir = options.ledger_dir or root / "Verify/optimization/stages"
    try:
        if options.list:
            print(encode(queue(root, ledger_dir, options.include_unverified)).decode(), end="")
        elif options.check_bundle is not None:
            print(encode(check_bundle(options.check_bundle)).decode(), end="")
        elif options.prepare:
            if options.output_dir is None:
                parser.error("--prepareには--output-dirが必要です")
            print(encode(prepare(root, options.prepare, options.output_dir, stage=options.stage,
                                 codex_receipt=options.codex_receipt)).decode(), end="")
        else:
            print(record_stage(root, ledger_dir, options.record_stage, options.codex_receipt))
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
