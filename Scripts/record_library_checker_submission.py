#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""公開したLibrary Checker提出の本文を照合し、明示指定時だけ判定記録へ保存する。"""
from __future__ import annotations

import sys
sys.dont_write_bytecode = True

import argparse
import copy
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import re
import tempfile
import urllib.error
import urllib.request
from urllib.parse import urlsplit

try:
    from . import update_verify_status as verify_status
except ImportError:
    import update_verify_status as verify_status

ROOT = Path(__file__).resolve().parents[1]
MAX_RESPONSE = 2 * 1024 * 1024
MAX_SOURCE = 512 * 1024
TERMINAL = frozenset({"AC", "WA", "RE", "TLE", "MLE", "OLE", "CE", "IE"})
SUBMISSION_URL = re.compile(r"https://judge\.yosupo\.jp/submission/([1-9][0-9]{0,18})\Z")
PROBLEM_PATH = re.compile(r"/problem/([A-Za-z0-9_]+)\Z")


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def submission_id(url: str) -> int:
    match = SUBMISSION_URL.fullmatch(url)
    if not match or int(match[1]) > (1 << 63) - 1:
        raise ValueError("公式の提出URL https://judge.yosupo.jp/submission/整数 を指定してください")
    return int(match[1])


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, request, file, code, message, headers, new_url):
        raise ValueError("公式APIからのリダイレクトは許可しません")


def fetch_response(number: int) -> bytes:
    # URLの入力値を接続先として使わず、確認済みの公式エンドポイントだけを構成する。
    endpoint = f"https://v3.api.judge.yosupo.jp/submissions/{number}"
    request = urllib.request.Request(endpoint, headers={
        "Accept": "application/json", "Accept-Encoding": "identity", "User-Agent": "sirkit-verify-import/1"})
    opener = urllib.request.build_opener(NoRedirect())
    with opener.open(request, timeout=30) as response:
        if response.geturl() != endpoint or response.status != 200:
            raise ValueError("公式APIの応答URLまたはHTTP状態が不正です")
        if response.headers.get_content_type() != "application/json":
            raise ValueError("公式APIの応答がJSONではありません")
        content = response.read(MAX_RESPONSE + 1)
    if len(content) > MAX_RESPONSE:
        raise ValueError("API応答がサイズ上限を超えました")
    return content


def unique_object(pairs):
    value = {}
    for name, item in pairs:
        if name in value:
            raise ValueError("JSONのキーが重複しています: " + name)
        value[name] = item
    return value


def parse_response(content: bytes) -> dict:
    if not content or len(content) > MAX_RESPONSE:
        raise ValueError("API応答が空、またはサイズ上限を超えています")
    value = json.loads(content.decode("utf-8"), object_pairs_hook=unique_object)
    if not isinstance(value, dict) or not {"overview", "source", "case_results"} <= value.keys():
        raise ValueError("未対応のAPI応答です。overview・source・case_resultsが必要です")
    return value


def nonnegative_number(value: object, name: str) -> int | float:
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError(name + "は有限の非負数が必要です")
    return value


def validate_response(response: dict, number: int, expected_problem: str) -> tuple[dict, bytes, list]:
    overview = response["overview"]
    if not isinstance(overview, dict):
        raise ValueError("overviewの形式が不正です")
    if type(overview.get("id")) is not int or overview["id"] != number:
        raise ValueError("提出URLとAPI応答の提出番号が一致しません")
    if overview.get("problem_name") != expected_problem:
        raise ValueError("指定したVerifyと提出先の問題が一致しません")
    result = overview.get("status")
    if not isinstance(result, str) or result not in TERMINAL:
        raise ValueError("未確定または未対応の判定です。確定後に取り込んでください")
    if (not isinstance(overview.get("lang"), str)
            or not re.fullmatch(r"[A-Za-z0-9_-]{1,64}", overview["lang"])):
        raise ValueError("API応答の言語名が不正です")
    stamp = overview.get("submission_time")
    if not isinstance(stamp, str):
        raise ValueError("API応答に提出時刻がありません")
    try:
        parsed = datetime.fromisoformat(stamp.replace("Z", "+00:00"))
    except ValueError:
        raise ValueError("API応答の提出時刻が不正です") from None
    if parsed.tzinfo is None:
        raise ValueError("API応答の提出時刻にタイムゾーンがありません")
    if overview.get("time") is not None:
        nonnegative_number(overview["time"], "実行時間")
    if overview.get("memory") is not None:
        if type(overview["memory"]) is not int or overview["memory"] < 0:
            raise ValueError("API応答のメモリ量が不正です")
    if result == "AC" and (overview.get("time") is None or overview.get("memory") is None):
        raise ValueError("ACの時間・メモリ情報が不足しています")
    source = response["source"]
    if not isinstance(source, str) or not source or "\0" in source:
        raise ValueError("API応答に提出ソース全文がありません")
    uploaded = source.encode("utf-8")
    if len(uploaded) > MAX_SOURCE:
        raise ValueError("提出ソースが512 KiBを超えています")
    cases = response["case_results"]
    if cases is None and result != "AC":
        cases = []
    if not isinstance(cases, list) or (result == "AC" and not cases):
        raise ValueError("API応答のケース結果が不足しています")
    for case in cases:
        if (not isinstance(case, dict) or not isinstance(case.get("case"), str)
                or not case["case"] or not isinstance(case.get("status"), str)
                or case["status"] not in TERMINAL):
            raise ValueError("API応答のケース結果が未確定または不正です")
        if result == "AC" and case["status"] != "AC":
            raise ValueError("AC判定と各ケースの判定が矛盾しています")
        if case.get("time") is not None:
            nonnegative_number(case["time"], "ケース実行時間")
        if case.get("memory") is not None and (type(case["memory"]) is not int or case["memory"] < 0):
            raise ValueError("ケースのメモリ量が不正です")
    return overview, uploaded, cases


def metadata_normalization_safe(source: bytes) -> bool:
    return b"/*" not in source and re.search(rb'\b(?:u8|u|U|L)?R"|\\\r?\n', source) is None


def compare_sources(current: bytes, uploaded: bytes) -> str:
    if current == uploaded:
        return "exact"
    if current.replace(b"\r\n", b"\n") == uploaded.replace(b"\r\n", b"\n"):
        return "line_endings"
    # 両方とも行単位の正規化が安全な場合だけ、既存の限定的な正規化を使う。
    if metadata_normalization_safe(current) and metadata_normalization_safe(uploaded):
        proof = {"uploaded_source_sha256": digest(uploaded),
                 "normalized_source_sha256": digest(verify_status.normalized_source(uploaded)),
                 "source_normalization": "submission-metadata-v1"}
        if verify_status.source_match(proof, current) == "submission_metadata_normalized":
            return "submission_metadata_normalized"
    raise ValueError("提出ソース全文が現行の本体・main・共通入出力と一致しません。記録は変更しません")


def load_journal(path: Path) -> tuple[dict, bytes | None]:
    before = path.read_bytes() if path.exists() else None
    value = json.loads(before.decode("utf-8"), object_pairs_hook=unique_object) if before is not None else {"entries": []}
    if not isinstance(value, dict) or "entries" not in value:
        raise ValueError("judgements.jsonの形式が不正です")
    # 既存フィールドは変更しないが、不正なID・重複・ハッシュを含む記録へ上書きしない。
    verify_status.records_from(value["entries"])
    return value, before


def prepare_import(root: Path, identifier: str, url: str, response_content: bytes,
                   *, fetched_live: bool = False, checked_at: str | None = None) -> dict:
    number = submission_id(url)
    if not verify_status.IDENTIFIER.fullmatch(identifier):
        raise ValueError("Verify IDの形式が不正です")
    response = parse_response(response_content)
    with verify_status.repository(root):
        matches = [entry for entry in verify_status.prepare_verify.collect() if entry["id"] == identifier]
        if len(matches) != 1:
            raise ValueError("指定したVerify IDが見つかりません")
        entry = matches[0]
        parts = urlsplit(entry.get("problem_url") or "")
        problem = PROBLEM_PATH.fullmatch(parts.path)
        if parts.scheme != "https" or parts.netloc != "judge.yosupo.jp" or not problem or parts.query or parts.fragment:
            raise ValueError("このVerify IDはLibrary Checkerの問題ではありません")
        overview, uploaded, cases = validate_response(response, number, problem[1])
        current, source_hashes = verify_status.prepare_verify.render(entry)
        comparison = compare_sources(current, uploaded)
        driver = (root / entry["driver"]).read_bytes()
        support = verify_status.prepare_verify.common_source_hashes(driver.decode())
    record = {"id": identifier, "status": overview["status"], "url": url,
              "submission_sha256": digest(current), "uploaded_source_sha256": digest(uploaded),
              "source_hashes": source_hashes, "driver_sha256": digest(driver),
              "support_source_hashes": support,
              "io_source_hashes": {verify_status.prepare_verify.FAST_IO: support[verify_status.prepare_verify.FAST_IO]}
              if verify_status.prepare_verify.FAST_IO in support else {},
              "io_mode": ("shared_fast_io" if verify_status.prepare_verify.FAST_IO_INCLUDE in driver.decode().splitlines()
                          else "custom_online_io" if verify_status.prepare_verify.custom_io_enabled(entry)
                          else "local_driver"),
              "source_match_method": comparison,
              "source_match": {"exact": "提出全文と現行生成コードがバイト単位で一致。",
                               "line_endings": "CRLF/LFのみ正規化後、提出全文と現行生成コードが一致。",
                               "submission_metadata_normalized": "CRLF/LF・空行・行端空白・Submissionコメントのみを正規化後、提出全文と現行生成コードが一致。"}[comparison],
              "matched_current_generation": True, "needs_resubmission": overview["status"] != "AC",
              "needs_performance_resubmission": overview["status"] != "AC",
              "needs_source_confirmation": False,
              "language": overview["lang"], "submission_time": overview["submission_time"],
              "checked_at_utc": checked_at or datetime.now(timezone.utc).isoformat(),
              "evidence_kind": "public_judge_api" if fetched_live else "supplied_api_response",
              "api_endpoint": f"https://v3.api.judge.yosupo.jp/submissions/{number}",
              "response_sha256": digest(response_content), "case_results": copy.deepcopy(cases)}
    if overview.get("time") is not None:
        record["time_ms"] = overview["time"] * 1000
    if overview.get("memory") is not None:
        record["memory_bytes"] = overview["memory"]
    if isinstance(overview.get("is_latest"), bool):
        record["is_latest"] = overview["is_latest"]
    # 判定の記録後にSubmissionコメントを最新URLへ更新しても、同じ本文を照合できるようにする。
    if metadata_normalization_safe(current) and metadata_normalization_safe(uploaded):
        assert verify_status.normalized_source(current) == verify_status.normalized_source(uploaded)
        record["source_normalization"] = "submission-metadata-v1"
        record["normalized_source_sha256"] = digest(verify_status.normalized_source(uploaded))
    journal_path = root / "Verify/current/judgements.json"
    journal, before = load_journal(journal_path)
    entries = journal["entries"]
    old = next((r for r in entries if r["id"] == identifier), None)
    identical = old is not None and all(old.get(key) == record.get(key) for key in (
        "status", "url", "submission_sha256", "uploaded_source_sha256", "response_sha256",
        "source_normalization", "normalized_source_sha256"))
    if not identical:
        if old is not None:
            record["history"] = [copy.deepcopy(old)]
            entries[entries.index(old)] = record
        else:
            entries.append(record)
    encoded = (json.dumps(journal, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode()
    return {"record": old if identical else record, "changed": not identical,
            "journal": encoded, "before": before, "path": journal_path}


def write_import(proposal: dict) -> None:
    if not proposal["changed"]:
        return
    path = proposal["path"]
    current = path.read_bytes() if path.exists() else None
    if current != proposal["before"]:
        raise ValueError("確認中にjudgements.jsonが変更されました。再実行してください")
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix=".judgements-", suffix=".tmp", delete=False) as output:
            temporary = Path(output.name)
            if path.exists():
                os.fchmod(output.fileno(), path.stat().st_mode & 0o777)
            output.write(proposal["journal"])
            output.flush()
            os.fsync(output.fileno())
        if (path.read_bytes() if path.exists() else None) != proposal["before"]:
            raise ValueError("保存直前にjudgements.jsonが変更されました。再実行してください")
        os.replace(temporary, path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--id", required=True, dest="identifier")
    parser.add_argument("--url", required=True)
    parser.add_argument("--response-json", type=Path, help="保存した公式API応答を使用。ネットワーク接続しない")
    parser.add_argument("--write", action="store_true", help="照合に成功した記録を保存する。省略時は確認のみ")
    args = parser.parse_args()
    try:
        number = submission_id(args.url)
        if args.response_json:
            with args.response_json.open("rb") as file:
                content = file.read(MAX_RESPONSE + 1)
        else:
            content = fetch_response(number)
        proposal = prepare_import(args.root.resolve(), args.identifier, args.url, content,
                                  fetched_live=args.response_json is None)
        if args.write:
            write_import(proposal)
        print(json.dumps({"written": bool(args.write and proposal["changed"]),
                          "would_change": proposal["changed"], "record": proposal["record"]},
                         ensure_ascii=False, indent=2, allow_nan=False))
    except (ValueError, OSError, urllib.error.URLError) as error:
        parser.exit(2, str(error) + "\n")


if __name__ == "__main__":
    main()
