#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""現行の提出全文と過去の判定を照合し、Verify状況とREADMEの集計を生成する。"""
from __future__ import annotations

import sys
sys.dont_write_bytecode = True

import argparse
from collections import Counter
from contextlib import contextmanager
import hashlib
import html
import json
from pathlib import Path
import re
from urllib.parse import quote, urlsplit

try:
    from . import prepare_verify
except ImportError:
    import prepare_verify

ROOT = Path(__file__).resolve().parents[1]
START = "<!-- VERIFY_STATUS:START -->"
END = "<!-- VERIFY_STATUS:END -->"
STATUSES = ("current_ac", "source_changed", "unsubmitted", "non_ac", "local_only")
LABELS = dict(current_ac="現行AC", source_changed="現行未照合・要再提出",
              unsubmitted="未提出", non_ac="非AC", local_only="ローカル補足")
SHA = re.compile(r"[0-9a-fA-F]{64}\Z")
IDENTIFIER = re.compile(r"[A-Za-z0-9_-]+\Z")
PREAMBLE_NORMALIZATION = "generated-preamble-metadata-v2"


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def markdown(value: object) -> str:
    text = " ".join(str(value).split())
    text = html.escape(text, quote=False)
    return re.sub(r"([\\`*_{}\[\]()#!|])", r"\\\1", text)


def safe_url(url: object, *, judge: str | None = None) -> str:
    if not isinstance(url, str) or any(c.isspace() or ord(c) < 32 for c in url):
        raise ValueError("判定URLの形式が不正です")
    parts = urlsplit(url)
    if (parts.scheme != "https" or parts.hostname not in prepare_verify.ONLINE_JUDGE_NAMES
            or parts.username or parts.password or parts.port not in (None, 443)
            or (judge is not None and
                prepare_verify.ONLINE_JUDGE_NAMES.get(parts.hostname) !=
                prepare_verify.ONLINE_JUDGE_NAMES.get(judge))):
        raise ValueError("判定URLの提出先が不正です")
    return quote(url, safe=":/?=&%#._~-")


def normalized_source(data: bytes) -> bytes:
    """過去に照合された限定的な正規化。C++の一般コメントは除かない。"""
    text = data.decode("utf-8").replace("\r\n", "\n")
    lines = [line.rstrip(" \t") for line in text.split("\n")
             if line.strip(" \t") and not line.startswith("// Submission:")]
    return "\n".join(lines).encode("utf-8")


def normalized_preamble_source(data: bytes) -> bytes | None:
    """定型冒頭の注記だけを除く。本文はCRLF例外以外バイト単位で保持する。"""
    raw = re.search(rb'\b(?:u8|u|U|L)?R"', data)
    # 行数を観測する機能・行継続・別表記の前処理は保守的に拒否する。
    if (b"\0" in data or re.search(
            rb'\\[ \t\v\f]*\r?\n|\?\?/|%:|##|\r(?!\n)|'
            rb'\b(?:__LINE__|__FILE__|__FILE_NAME__|__BASE_FILE__|__builtin_LINE|__builtin_FILE|'
            rb'__builtin_source_location|source_location|stacktrace)\b|'
            rb'(?:^|\n)[ \t]*#[^\n]*\b(?:line|embed)\b', data)):
        return None
    if b"\r\n" in data:
        # raw文字列の物理改行については同値性を仮定しない。
        if raw:
            return None
        data = data.replace(b"\r\n", b"\n")
    return _remove_preamble_metadata(data)


def has_generated_preamble(data: bytes) -> bool:
    """形式の認識だけに使う。安全性の判定や正規化証拠には使わない。"""
    return _remove_preamble_metadata(data.replace(b"\r\n", b"\n")) is not None


def _remove_preamble_metadata(data: bytes) -> bytes | None:
    lines = data.splitlines(keepends=True)
    kept: list[bytes] = []
    seen_verify = False
    index = 0
    while index < len(lines):
        line = lines[index]
        text = line.removesuffix(b"\n")
        if text.startswith(b"// Library: "):
            if not seen_verify:
                return None
            # 最初のライブラリ境界以降はコメントも含め完全にopaqueにする。
            return b"".join(kept) + b"".join(lines[index + 1:])
        if text.startswith(b"// Verify: "):
            if seen_verify:
                return None
            seen_verify = True
        elif text.startswith(b"// Submission: "):
            pass
        elif (not text.strip(b" \t") or text.lstrip(b" \t").startswith(b"//")
              or text in (b"#define SKIP_RUNTIME_CHECKS", b"#define NDEBUG")):
            kept.append(line)
        elif text == b"#if defined(__GNUC__) && !defined(__clang__)":
            if index + 2 >= len(lines):
                return None
            pragma, end = (part.removesuffix(b"\n") for part in lines[index + 1:index + 3])
            if (not re.fullmatch(rb'#pragma GCC optimize\("[A-Za-z0-9=_-]+"(?:, "[A-Za-z0-9=_-]+")*\)', pragma)
                    or end != b"#endif"):
                return None
            kept.extend(lines[index:index + 3])
            index += 2
        else:
            # include、macro、raw文字列等を越えて注記を探さない。
            return None
        index += 1
    return None


def preamble_source_proof(uploaded: bytes) -> dict | None:
    """実際に取得した提出全文だけから追加証拠を作る。現行SHAは使わない。"""
    normalized = normalized_preamble_source(uploaded)
    if normalized is None:
        return None
    return {"normalization": PREAMBLE_NORMALIZATION,
            "uploaded_source_sha256": digest(uploaded),
            "normalized_source_sha256": digest(normalized)}


def normalization_matches(scheme: str, expected: object, source: bytes) -> bool:
    """判定証拠と確認待ち期待値で共用する限定的な本文照合。"""
    if not isinstance(expected, str) or not SHA.fullmatch(expected):
        return False
    if scheme == PREAMBLE_NORMALIZATION:
        normalized = normalized_preamble_source(source)
        return normalized is not None and digest(normalized) == expected.lower()
    if scheme == "submission-metadata-v1":
        if b'/*' in source or re.search(rb'\b(?:u8|u|U|L)?R"|\\\r?\n', source):
            return False
        normalized = normalized_source(source)
        return expected.lower() in {digest(normalized), digest(normalized + b"\n")}
    return False


def preamble_proof_matches(record: dict, source: bytes) -> bool:
    uploaded = record.get("uploaded_source_sha256")
    proofs = record.get("source_normalization_proofs")
    if not isinstance(uploaded, str) or not SHA.fullmatch(uploaded) or not isinstance(proofs, list):
        return False
    for proof in proofs:
        if not isinstance(proof, dict):
            continue
        actual = proof.get("uploaded_source_sha256")
        if (proof.get("normalization") == PREAMBLE_NORMALIZATION
                and isinstance(actual, str) and actual.lower() == uploaded.lower()
                and normalization_matches(PREAMBLE_NORMALIZATION,
                                          proof.get("normalized_source_sha256"), source)):
            return True
    return False


def normalized_proof(record: dict) -> bool:
    if not record.get("uploaded_source_sha256") or not record.get("normalized_source_sha256"):
        return False
    if record.get("source_normalization") == "submission-metadata-v1":
        return True
    explanation = record.get("source_match", "")
    if not isinstance(explanation, str) or "Submission" not in explanation:
        return False
    # 全文コメント削除や機械語の一致は、この正規化の根拠には使わない。
    if any(word in explanation.lower() for word in (
            "全文コメント", "全コメント", "all comments", "full-line comments", "machine code")):
        return False
    return ("正規化" in explanation or "normaliz" in explanation.lower()) and (
        "のみ" in explanation or "only" in explanation.lower())


def source_match(record: dict, source: bytes) -> str | None:
    current = digest(source)
    if current in {record.get("submission_sha256"), record.get("uploaded_source_sha256")}:
        return "exact"
    if "source_normalization_proofs" in record:
        # v2証拠を持つ記録は、より広い旧v1へフォールバックしない。
        return "generated_preamble_metadata_normalized" if preamble_proof_matches(record, source) else None
    # raw string・行継続・ブロックコメントでは、行単位の除去が本文を変え得る。
    # この場合は保守的に全文の完全一致だけを使う。
    if normalized_proof(record) and normalization_matches(
            "submission-metadata-v1", record["normalized_source_sha256"], source):
        return "submission_metadata_normalized"
    return None


def records_from(value: object, *, nested: bool = False) -> list[dict]:
    if not isinstance(value, list):
        raise ValueError("judgementsのentries/historyは配列が必要です")
    output = []
    seen = set()
    for item in value:
        if not isinstance(item, dict):
            raise ValueError("判定記録はオブジェクトが必要です")
        if nested and "status" not in item:
            # 過去のjournalには判定をrecordで包む履歴と、ローカル状態だけの注記もある。
            if "record" in item:
                output.extend(records_from([item["record"]], nested=True))
            continue
        record = dict(item)
        name = record.get("id")
        if name is not None and (not isinstance(name, str) or not IDENTIFIER.fullmatch(name)):
            raise ValueError("判定IDの形式が不正です")
        if not nested:
            if name is None or name in seen:
                raise ValueError("判定IDの欠落または重複があります")
            seen.add(name)
        if not isinstance(record.get("status"), str) or not record["status"]:
            raise ValueError("判定statusが必要です")
        for key in ("submission_sha256", "uploaded_source_sha256", "normalized_source_sha256"):
            if record.get(key) is not None:
                if not isinstance(record[key], str) or not SHA.fullmatch(record[key]):
                    raise ValueError("判定ソースのSHA-256が不正です: " + key)
                record[key] = record[key].lower()
        if "source_normalization_proofs" in record:
            proofs = record["source_normalization_proofs"]
            if not isinstance(proofs, list):
                raise ValueError("提出冒頭の正規化証拠は配列が必要です")
            for proof in proofs:
                if (not isinstance(proof, dict) or proof.get("normalization") != PREAMBLE_NORMALIZATION
                        or any(not isinstance(proof.get(key), str) or not SHA.fullmatch(proof[key])
                               for key in ("uploaded_source_sha256", "normalized_source_sha256"))
                        or proof["uploaded_source_sha256"].lower() != record.get("uploaded_source_sha256")):
                    raise ValueError("提出冒頭の正規化証拠と実提出SHAが一致しません")
        if record.get("url") is not None:
            safe_url(record["url"])
        elapsed = record.get("time_ms")
        if elapsed is not None and (isinstance(elapsed, bool) or not isinstance(elapsed, (int, float))
                                    or not 0 <= elapsed < float("inf")):
            raise ValueError("判定時間が不正です")
        if "history" in record:
            record["history"] = records_from(record["history"], nested=True)
        output.append(record)
    return output


def history(record: dict) -> list[dict]:
    result = [record]
    for old in reversed(record.get("history", [])):
        result.extend(history(old))
    return result


@contextmanager
def repository(root: Path):
    previous_root, previous_base = prepare_verify.ROOT, prepare_verify.BASE
    prepare_verify.ROOT, prepare_verify.BASE = root, root / "Verify/current"
    try:
        yield
    finally:
        prepare_verify.ROOT, prepare_verify.BASE = previous_root, previous_base


def submission_policy(root: Path, entries: list[dict]) -> dict[str, dict]:
    """再提出免除・参考検証の扱い。判定やACの本文照合は変更しない。"""
    path = root / "Verify/current/submission_policy.json"
    if not path.exists():
        return {}
    def unique_object(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError("提出方針のJSONキーが重複しています: " + key)
            result[key] = value
        return result

    value = json.loads(path.read_text(), object_pairs_hook=unique_object)
    if (not isinstance(value, dict) or type(value.get("format")) is not int
            or value.get("format") != 1 or not isinstance(value.get("entries"), list)):
        raise ValueError("提出方針の形式が不正です")
    known = {e["id"] for e in entries}
    result = {}
    for rule in value["entries"]:
        if (not isinstance(rule, dict) or not isinstance(rule.get("id"), str)
                or not IDENTIFIER.fullmatch(rule["id"])
                or rule["id"] not in known or rule["id"] in result):
            raise ValueError("提出方針のIDが不明または重複しています")
        if not isinstance(rule.get("reason"), str) or not rule["reason"].strip():
            raise ValueError("提出方針の理由が必要です")
        if rule.get("kind") == "documentation_only":
            if not isinstance(rule.get("source_sha256"), str) or not SHA.fullmatch(rule["source_sha256"]):
                raise ValueError("説明変更免除には当該ソースSHAが必要です")
        elif rule.get("kind") == "reference":
            replacements = rule.get("replacements")
            if (not isinstance(replacements, list) or not replacements
                    or any(not isinstance(i, str) or i not in known or i == rule["id"] for i in replacements)
                    or len(set(replacements)) != len(replacements)):
                raise ValueError("参考検証には有効な移行先IDが必要です")
        else:
            raise ValueError("未知の提出方針です")
        result[rule["id"]] = rule
    # 参考同士の循環や、最終的な主検証を持たない移行先を許さない。
    for rule in result.values():
        if rule["kind"] == "reference" and any(result.get(i, {}).get("kind") == "reference" for i in rule["replacements"]):
            raise ValueError("参考検証の移行先は主検証である必要があります")
    return result


def submission_action(rule: dict | None, source_sha256: str) -> dict:
    if rule is None:
        return {}
    if rule["kind"] not in {"documentation_only", "reference"}:
        raise ValueError("未知の提出方針です")
    if rule["kind"] == "documentation_only" and (
            not isinstance(source_sha256, str) or not SHA.fullmatch(source_sha256)
            or rule["source_sha256"].lower() != source_sha256.lower()):
        return {}
    return {"kind": rule["kind"], "reason": rule["reason"],
            **({"replacements": list(rule["replacements"])} if rule["kind"] == "reference" else {})}


def build_status(root: Path) -> dict:
    journal = root / "Verify/current/judgements.json"
    data = json.loads(journal.read_text()) if journal.exists() else {"entries": []}
    if not isinstance(data, dict) or "entries" not in data:
        raise ValueError("judgementsはentriesを持つオブジェクトが必要です")
    by_id = {r["id"]: r for r in records_from(data["entries"])}
    rows = []
    with repository(root):
        entries = prepare_verify.collect()
        policies = submission_policy(root, entries)
        for entry in sorted(entries, key=lambda e: e["id"]):
            name = entry["id"]
            if not IDENTIFIER.fullmatch(name):
                raise ValueError("Verify IDの形式が不正です")
            source, hashes = prepare_verify.render(entry)
            problem = entry.get("problem_url")
            host = urlsplit(problem).hostname if problem else None
            if problem:
                safe_url(problem)
            record = by_id.get(name)
            candidates = history(record) if record else []
            match = None
            matched = None
            for candidate in candidates:
                if candidate.get("id", name) != name:
                    continue
                # 別問題の記録をこの提出の根拠にしない。
                if candidate.get("url"):
                    safe_url(candidate["url"])
                    if (prepare_verify.ONLINE_JUDGE_NAMES.get(urlsplit(candidate["url"]).hostname)
                            != prepare_verify.ONLINE_JUDGE_NAMES.get(host)):
                        continue
                match = source_match(candidate, source) if candidate.get("url") else None
                if match:
                    matched = candidate
                    break
            if not problem:
                status = "local_only"
            elif matched:
                status = "current_ac" if matched["status"] == "AC" else "non_ac"
            elif any(r["status"] == "AC" for r in candidates):
                status = "source_changed"
            elif candidates:
                status = "non_ac"
            else:
                status = "unsubmitted"
            row = dict(id=name, title=entry.get("title", name), status=status,
                       judge=host or "local", problem_url=problem,
                       libraries=entry["libraries"], source_hashes=hashes,
                       submission="Verify/current/submissions/" + name + ".cpp",
                       submission_sha256=digest(source))
            action = submission_action(policies.get(name), row["submission_sha256"])
            row["submission_required"] = bool(problem) and status not in ("current_ac", "local_only") and not action
            if action:
                row["submission_policy"] = action
            submission = root / row["submission"]
            row["submission_exists"] = submission.is_file()
            row["submission_in_sync"] = submission.is_file() and submission.read_bytes() == source
            evidence = matched if matched else record
            if evidence:
                row["judgement"] = {key: evidence[key] for key in (
                    "status", "url", "submission_sha256", "uploaded_source_sha256") if key in evidence}
                if matched:
                    row["judgement"]["source_match"] = match
                    if evidence.get("time_ms") is not None:
                        row["judgement"]["time_ms"] = evidence["time_ms"]
            rows.append(row)
    known = {e["id"] for e in entries}
    counts = {status: sum(r["status"] == status for r in rows) for status in STATUSES}
    return dict(format=1, profile="normal", entries=rows, counts=counts,
                orphan_judgement_ids=sorted(set(by_id) - known))


def status_markdown(data: dict) -> str:
    lines = ["# Verify状況", "", "通常版の現在の本体・main・共通入出力を含む提出全文と、保存した判定記録を照合します。",
             "現行ACはソースが一致するACだけです。過去のACやローカル検査を現行ACへ数えません。",
             "提出ファイルが未生成・未同期の場合は、先に `python3 Scripts/prepare_verify.py` を実行してください。", "",
             "更新: `python3 Scripts/update_verify_status.py`。オンライン情報を自動取得する処理ではありません。", "",
             "| ID | 提出先 | 状況 | 判定 | 現行一致の時間 |", "| --- | --- | --- | --- | --- |"]
    for row in data["entries"]:
        judge = "ローカル" if row["judge"] == "local" else prepare_verify.ONLINE_JUDGE_NAMES[row["judge"]]
        target = (f"[{judge}]({safe_url(row['problem_url'])})" if row["problem_url"] else judge)
        evidence = row.get("judgement", {})
        label = markdown(evidence.get("status", "—"))
        if evidence.get("url"):
            label = f"[{label}]({safe_url(evidence['url'])})"
        elapsed = f"{evidence['time_ms']:g} ms" if "time_ms" in evidence else "—"
        identifier = markdown(row["id"])
        if row["submission_exists"]:
            identifier = f"[{identifier}](submissions/{quote(row['id'])}.cpp)"
        action = row.get("submission_policy", {})
        condition = ("説明変更のみ・再提出不要" if action.get("kind") == "documentation_only" else
                     "参考検証・移行先あり" if action.get("kind") == "reference" else LABELS[row["status"]])
        condition += "（提出ファイル未同期）" if not row["submission_in_sync"] else ""
        lines.append(f"| {identifier} | {target} | {condition} | {label} | {elapsed} |")
    if data["orphan_judgement_ids"]:
        lines.extend(["", "現行のVerify一覧にない過去の判定ID: " + ", ".join(
            markdown(name) for name in data["orphan_judgement_ids"]) + "。集計には含めません。"])
    return "\n".join(lines) + "\n"


def readme_summary(data: dict) -> str:
    lines = ["[Verify状況の一覧](Verify/current/status.md)（通常版の現行ソースと保存した判定を照合）", "",
             "| 提出先 | 対象 | 現行AC | 現行未照合 | 未提出 | 非AC |",
             "| --- | ---: | ---: | ---: | ---: | ---: |"]
    for hosts, name in ((("judge.yosupo.jp",), "Library Checker"),
                        (("onlinejudge.u-aizu.ac.jp", "judge.u-aizu.ac.jp"), "AOJ"),
                        (("yukicoder.me",), "Yukicoder"), (("atcoder.jp",), "AtCoder")):
        counts = Counter(r["status"] for r in data["entries"] if r["judge"] in hosts)
        values = [sum(counts.values()), *(counts[s] for s in STATUSES[:-1])]
        lines.append("| " + name + " | " + " | ".join(map(str, values)) + " |")
    exempt = sum(r.get("submission_policy", {}).get("kind") == "documentation_only" for r in data["entries"])
    reference = sum(r.get("submission_policy", {}).get("kind") == "reference" for r in data["entries"])
    if exempt or reference:
        lines.extend(["", f"説明変更のみの再提出免除: {exempt}件、移行前の参考検証: {reference}件。いずれも再提出対象から除き、現行ACへの新規照合とは区別します。"])
    lines.extend(["", f"ローカル補足: {data['counts']['local_only']}件。過去のAC・ローカル結果は現行ACと区別します。"])
    return "\n".join(lines) + "\n"


def update_readme(text: str, summary: str) -> str:
    if text.count(START) != 1 or text.count(END) != 1:
        raise ValueError("READMEにはVERIFY_STATUSの開始・終了マーカーを1組置いてください")
    begin, end = text.index(START) + len(START), text.index(END)
    if begin > end:
        raise ValueError("READMEのVERIFY_STATUSマーカー順が不正です")
    return text[:begin] + "\n" + summary + text[end:]


def generate(root: Path, *, readme: str = "Readme.md", status_only: bool = False) -> dict[Path, bytes]:
    data = build_status(root)
    outputs = {root / "Verify/current/status.json": (json.dumps(data, ensure_ascii=False, indent=2) + "\n").encode(),
               root / "Verify/current/status.md": status_markdown(data).encode()}
    if not status_only:
        target = root / readme
        if not target.resolve().is_relative_to(root.resolve()) or not target.is_file():
            raise ValueError("READMEはリポジトリ内の既存ファイルを指定してください")
        outputs[target] = update_readme(target.read_text(), readme_summary(data)).encode()
    return outputs


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--readme", default="Readme.md")
    parser.add_argument("--status-only", action="store_true", help="READMEを変更せず一覧だけ生成")
    parser.add_argument("--check", action="store_true", help="生成物の一致だけを確認")
    args = parser.parse_args()
    outputs = generate(args.root.resolve(), readme=args.readme, status_only=args.status_only)
    mismatches = [str(path) for path, value in outputs.items() if not path.is_file() or path.read_bytes() != value]
    if args.check:
        if mismatches:
            raise SystemExit("Verify状況が未同期です: " + ", ".join(mismatches))
        print("Verify状況: 同期確認済み")
        return
    for path, value in outputs.items():
        if not path.is_file() or path.read_bytes() != value:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(value)
    print("Verify状況: 更新済み")


if __name__ == "__main__":
    main()
