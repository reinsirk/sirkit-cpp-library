#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""通常版を実体化してclang-tidyの指摘を集計し、重大項目をgateにする。"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

try:
    from . import prepare_verify
except ImportError:
    import prepare_verify


ROOT = Path(__file__).resolve().parents[1]
LIBRARY_DIRS = frozenset({"Algorithm", "DataStructure", "Graph", "Math", "String", "Geometry", "Internal"})
CORE_STRICT_CHECKS = frozenset({
    "cppcoreguidelines-avoid-capturing-lambda-coroutines",
    "cppcoreguidelines-no-malloc", "cppcoreguidelines-owning-memory",
    "cppcoreguidelines-slicing", "cppcoreguidelines-virtual-class-destructor",
    "bugprone-use-after-move", "bugprone-unused-raii", "bugprone-suspicious-memset-usage",
})
GOOGLE_STRICT_CHECKS = frozenset({"google-build-using-namespace", "google-runtime-int", "google-runtime-operator"})
STRICT_CHECKS = CORE_STRICT_CHECKS | GOOGLE_STRICT_CHECKS
CORE_CHECKS = CORE_STRICT_CHECKS | frozenset({"cppcoreguidelines-interfaces-global-init",
                                           "cppcoreguidelines-pro-type-member-init", "performance-move-const-arg",
                                           "performance-unnecessary-copy-initialization", "performance-unnecessary-value-param"})
GOOGLE_CHECKS = frozenset({"google-build-explicit-make-pair", "google-build-using-namespace",
                           "google-readability-casting", "google-runtime-int", "google-runtime-operator"})
REQUIRED_CHECKS = CORE_CHECKS | GOOGLE_CHECKS
GUIDE_CHECKS = {"core": CORE_CHECKS, "google": GOOGLE_CHECKS, "both": REQUIRED_CHECKS}
GUIDE_STRICT_CHECKS = {"core": CORE_STRICT_CHECKS, "google": GOOGLE_STRICT_CHECKS, "both": STRICT_CHECKS}
GUIDE_TITLES = {"core": "C++ Core Guidelines", "google": "Google C++ Style Guide",
                "both": "C++ Core Guidelines / Google Style Guide"}
POSITION = re.compile(r"^(.+?):(\d+):(\d+): (warning|error|fatal error): (.*?)(?: \[([^\[\]]+)\])?$")
POLICY = {
    "reference": "https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines",
    "google_reference": "https://google.github.io/styleguide/cppguide.html",
    "tool": "https://clang.llvm.org/extra/clang-tidy/",
    "strict": "所有権・malloc・スライシング・仮想破棄・move後の使用に加え、using-directiveによる名前空間の汚染・固定幅でないlong/short整数・単項address-ofの独自演算子を失敗とする。",
    "report_only": "初期化・グローバル初期化・コピー・C形式cast・make_pairの明示型引数も表示するが、既存APIや段階的初期化を人が確認する。",
    "excluded_rules": "公開APIはsirkit、内部はsirkit::detail、既存geoはsirkit::geoとし、定数のk接頭辞（kInf）は採用済み。これらを判断待ちとは扱わない。一方、ACL互換の公開名の一括改名やファイル全体のheader guard強制など、部品別guard・既存API方針と異なる形式規則の全面適用は未選択。選択した13/5チェックは命名・namespace配置の網羅検査を含まない。添字・ポインタ演算・narrowing・magic numberの追加制約も別途影響を確認する。using-directive禁止の選択チェックと、公開APIをsirkitへ配置する決定は別物。",
    "scope": "通常版の原本7フォルダ。Fast/・Variants/・vendor・main/driver・連結提出版の警告は集計しない。",
    "limits": "Core Guidelines/Google Style Guideの全項目への準拠を保証しない。Verifyが実体化しないテンプレート/APIは静的解析にも現れない。",
}


def source_relative(path: Path, root: Path = ROOT) -> str | None:
    """生成版や外部ヘッダーを、同名の原本として数えない。"""
    try:
        relative = path.resolve().relative_to(root.resolve())
    except ValueError:
        return None
    return relative.as_posix() if relative.parts and relative.parts[0] in LIBRARY_DIRS else None


def parse_diagnostics(text: str, root: Path = ROOT, *, strict_checks: frozenset = STRICT_CHECKS) -> tuple[list[dict], list[dict]]:
    diagnostics, errors = [], []
    for line in text.splitlines():
        match = POSITION.match(line)
        if match is None:
            continue
        filename, row, column, severity, message, checks = match.groups()
        path = Path(filename)
        if not path.is_absolute():
            path = root / path
        relative = source_relative(path, root)
        item = {"path": relative or filename, "line": int(row), "column": int(column),
                "severity": severity, "message": message,
                "checks": checks.split(",") if checks else []}
        if severity in {"error", "fatal error"}:
            errors.append(item)
        if relative is not None:
            item["strict"] = bool(strict_checks.intersection(item["checks"]))
            diagnostics.append(item)
    return diagnostics, errors


def unique_diagnostics(items: list[dict]) -> list[dict]:
    result = {}
    for item in items:
        key = (item["path"], item["line"], item["column"], item["severity"],
               item["message"], tuple(item["checks"]))
        if key not in result:
            result[key] = dict(item, verify_ids=[])
        if item.get("verify_id") not in result[key]["verify_ids"]:
            result[key]["verify_ids"].append(item["verify_id"])
    return sorted(result.values(), key=lambda d: (d["path"], d["line"], d["column"], d["message"]))


def load_reviewed_exceptions(path: Path) -> list[dict]:
    data = json.loads(path.read_text())
    if not isinstance(data, dict) or type(data.get("format")) is not int or data["format"] != 1:
        raise ValueError("確認済み例外のformatが不正です")
    entries = data.get("entries")
    if not isinstance(entries, list):
        raise ValueError("確認済み例外のentriesが不正です")
    seen = set()
    for item in entries:
        if not isinstance(item, dict):
            raise ValueError("確認済み例外が不正です")
        name = item.get("path")
        if (not isinstance(name, str) or "\\" in name or name.split("/")[0] not in LIBRARY_DIRS
                or any(part in {"", ".", ".."} for part in name.split("/"))
                or not isinstance(item.get("check"), str) or item["check"] not in STRICT_CHECKS
                or not isinstance(item.get("source_sha256"), str)
                or re.fullmatch(r"[0-9a-f]{64}", item["source_sha256"]) is None
                or type(item.get("line")) is not int or item["line"] <= 0
                or type(item.get("column")) is not int or item["column"] <= 0
                or not isinstance(item.get("message"), str) or not item["message"]
                or not isinstance(item.get("reason"), str) or not item["reason"]):
            raise ValueError("確認済み例外の位置・SHA・理由が不正です")
        key = (name, item["line"], item["column"], item["check"], item["message"])
        if key in seen:
            raise ValueError("確認済み例外が重複しています")
        seen.add(key)
    return entries


def apply_reviewed_exceptions(items: list[dict], exceptions: list[dict], root: Path = ROOT) -> None:
    hashes = {}
    for item in items:
        item["reviewed_exception"] = None
        for exception in exceptions:
            if (item["path"] != exception["path"] or item["line"] != exception["line"]
                    or item["column"] != exception["column"]
                    or item["checks"] != [exception["check"]]
                    or item["message"] != exception["message"]):
                continue
            if item["path"] not in hashes:
                hashes[item["path"]] = hashlib.sha256((root / item["path"]).read_bytes()).hexdigest()
            if hashes[item["path"]] == exception["source_sha256"]:
                item["strict"] = False
                item["reviewed_exception"] = exception
                break


def checked_translation_unit(entry: dict, root: Path = ROOT) -> bytes:
    overrides = {}
    for name in entry["libraries"]:
        absolute = root / name
        if source_relative(absolute, root) != name:
            raise ValueError("通常版以外の本体は検査しません: " + name)
        overrides[name] = ('#include <' + str(absolute.resolve()) + '>\n').encode()
    data, _ = prepare_verify.render(entry, source_overrides=overrides, skip_runtime_checks=False)
    # driverが独自に検査省略を指定している場合は、checkedと偽らない。
    if re.search(rb"^\s*#\s*define\s+(SKIP_RUNTIME_CHECKS|NDEBUG)\b", data, re.M):
        raise ValueError("checked検査を無効にするdriver: " + entry["id"])
    return data


def validate_configuration(tidy: str, source: Path, output: Path, result: dict,
                           *, guide: str = "both") -> Path:
    """元設定の必須ruleと、固定したguide別の実効ruleを検証する。"""
    if guide not in GUIDE_CHECKS:
        raise ValueError("未知のguide: " + guide)
    required, strict = GUIDE_CHECKS[guide], GUIDE_STRICT_CHECKS[guide]
    data = source.read_bytes()
    original = output / "clang-tidy-source.yaml"
    original.write_bytes(data)
    fixed = output / "clang-tidy-checked.yaml"
    result.update({"status": "running", "guide": guide, "source": str(source), "used_config": str(fixed),
                   "sha256": hashlib.sha256(data).hexdigest(), "enabled_checks": [],
                   "required_checks": sorted(required)})

    def invoke(label: str, arguments: list[str]) -> str:
        command = [tidy, *arguments]
        process = subprocess.run(command, capture_output=True, text=True, timeout=30)
        result[label] = {"command": command, "returncode": process.returncode,
                         "stdout": process.stdout, "stderr": process.stderr}
        if process.returncode != 0:
            result["status"] = "FAIL"
            raise ValueError("clang-tidy設定を検証できません: " + label)
        return process.stdout

    def enabled(text: str) -> set[str]:
        return set(re.findall(r"^\s+([a-z][a-z0-9-]+)\s*$", text, re.M))

    invoke("source-verify-config", ["--verify-config", "--config-file=" + str(original)])
    available = enabled(invoke("source-list-checks", ["--list-checks", "--config-file=" + str(original)]))
    result["source_enabled_checks"] = sorted(available)
    missing = required - available
    result["missing_strict_checks"] = sorted(strict - available)
    result["missing_required_checks"] = sorted(missing)
    if missing:
        result["status"] = "FAIL"
        raise ValueError("必須チェックが有効ではありません: " + ", ".join(sorted(missing)))
    # Clang自身に設定を解決させる。中央設定をPython独自YAML解釈で改変しない。
    resolved = invoke("dump-config", ["--dump-config", "--config-file=" + str(original),
                                       "--checks=-*," + ",".join(sorted(required))])
    if not resolved.strip():
        result["status"] = "FAIL"
        raise ValueError("guide別の実効設定が空です")
    fixed.write_text(resolved)
    result["effective_sha256"] = hashlib.sha256(fixed.read_bytes()).hexdigest()
    invoke("verify-config", ["--verify-config", "--config-file=" + str(fixed)])
    active = enabled(invoke("list-checks", ["--list-checks", "--config-file=" + str(fixed)]))
    result["enabled_checks"] = sorted(active)
    result["missing_strict_checks"] = sorted(strict - active)
    result["missing_required_checks"] = sorted(required - active)
    result["unexpected_checks"] = sorted(active - required)
    if active != required:
        result["status"] = "FAIL"
        raise ValueError("guide別の実効チェックが一致しません")
    result["status"] = "PASS"
    return fixed


def compiler_options(tidy_version: str, *, gcc_install_dir: str | None = None,
                     root: Path = ROOT) -> list[str]:
    options = ["-std=c++23", "-I" + str(root), "-I" + str(root / "Verify/current/vendor")]
    major = re.search(r"(?:LLVM|clang) version (\d+)", tidy_version)
    clang = shutil.which("clang++-" + major[1]) if major else None
    if clang:
        resource = subprocess.run([clang, "-print-resource-dir"], capture_output=True, text=True, check=True)
        path = Path(resource.stdout.strip())
        if path.is_dir():
            options.append("-resource-dir=" + str(path))
    if gcc_install_dir:
        options.append("--gcc-install-dir=" + gcc_install_dir)
    return options


def markdown(report: dict) -> str:
    rows = report.get("runs", [])
    issues = report.get("diagnostics", [])
    strict = sum(d["strict"] for d in issues)
    reviewed = sum(bool(d.get("reviewed_exception")) for d in issues)
    lines = ["## " + GUIDE_TITLES.get(report.get("guide", "both"), "C++ Guidelines"), "", f"結果: **{report['status']}**", "",
             f"通常版Verify {len(rows)}件。重大な指摘 {strict}件、確認済み例外 {reviewed}件、参考の指摘 {len(issues) - strict - reviewed}件。", "",
             "Fast/・生成版・vendor・driverは警告の集計対象外です。両ガイドの全項目への準拠や未実体化APIの検証を保証しません。"]
    if issues:
        lines += ["", "| 区分 | 原本位置 | チェック |", "| --- | --- | --- |"]
        for item in issues[:30]:
            # 名称は原本パスと有効チェック名のみ。任意の診断文をHTMLへ流さない。
            path = re.sub(r"[^A-Za-z0-9_./() -]", "?", item["path"])
            checks = re.sub(r"[^A-Za-z0-9_,.-]", "?", ",".join(item["checks"]))
            kind = "重大" if item["strict"] else "確認済み例外" if item.get("reviewed_exception") else "参考"
            lines.append(f"| {kind} | `{path}:{item['line']}` | `{checks}` |")
        if len(issues) > 30:
            lines += ["", f"ほか{len(issues)-30}件。全文はJSONとrawログを参照してください。"]
    if report.get("error"):
        lines += ["", "検査を完了できませんでした。結果JSONのerrorと実行ログを確認してください。"]
    return "\n".join(lines) + "\n"


def save(output: Path, report: dict) -> None:
    output.mkdir(parents=True, exist_ok=True)
    (output / "result.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    (output / "summary.md").write_text(markdown(report))


def run(entries: list[dict], tidy: str, output: Path, gcc_install_dir: str | None = None,
        *, root: Path = ROOT, guide: str = "both") -> dict:
    active_strict = GUIDE_STRICT_CHECKS.get(guide, frozenset())
    report = {"format": 1, "status": "running", "guide": guide, "policy": POLICY,
              "strict_checks": sorted(active_strict), "runs": [], "diagnostics": [],
              "online_status": "not_evaluated"}
    output.mkdir(parents=True, exist_ok=True)
    (output / "raw").mkdir(exist_ok=True)
    save(output, report)
    try:
        if not entries:
            raise ValueError("検査対象がありません")
        if guide not in GUIDE_CHECKS:
            raise ValueError("未知のguide: " + guide)
        exceptions = []
        report["reviewed_exception_policy"] = {"entries": [], "sha256": None}
        if guide in {"core", "both"}:
            exception_path = root / "Scripts/ci_cpp_guidelines_exceptions.json"
            exceptions = load_reviewed_exceptions(exception_path)
            report["reviewed_exception_policy"] = {"entries": exceptions,
                                                   "sha256": hashlib.sha256(exception_path.read_bytes()).hexdigest()}
        version = subprocess.run([tidy, "--version"], capture_output=True, text=True, check=True)
        report["tool_version"] = version.stdout.strip()
        report["configuration"] = {}
        config = validate_configuration(tidy, root / ".clang-tidy", output, report["configuration"], guide=guide)
        options = compiler_options(version.stdout, gcc_install_dir=gcc_install_dir, root=root)
        header_filter = "^" + re.escape(str(root.resolve())) + "/(" + "|".join(sorted(LIBRARY_DIRS)) + ")/"
        diagnostics = []
        with tempfile.TemporaryDirectory(prefix="sirkit-guidelines-") as temporary:
            for entry in entries:
                source = checked_translation_unit(entry, root)
                path = Path(temporary) / (entry["id"] + ".cpp")
                path.write_bytes(source)
                command = [tidy, "--quiet", "--config-file=" + str(config),
                           "--header-filter=" + header_filter, str(path), "--", *options]
                result = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=180)
                text = result.stdout + result.stderr
                raw = output / "raw" / (entry["id"] + ".log")
                raw.write_text(text)
                items, errors = parse_diagnostics(text, root, strict_checks=active_strict)
                apply_reviewed_exceptions(items, exceptions, root)
                for item in items:
                    item["verify_id"] = entry["id"]
                diagnostics.extend(items)
                row = {"id": entry["id"], "command": command, "returncode": result.returncode,
                       "checked": True, "translation_unit_sha256": hashlib.sha256(source).hexdigest(),
                       "library_sha256": {name: hashlib.sha256((root/name).read_bytes()).hexdigest()
                                          for name in entry["libraries"]},
                       "raw_log": str(raw), "diagnostic_occurrences": len(items), "compiler_errors": errors,
                       "passed": result.returncode == 0 and not errors and not any(d["strict"] for d in items)}
                report["runs"].append(row)
                report["diagnostics"] = unique_diagnostics(diagnostics)
                save(output, report)
                print(f"{entry['id']}: {'PASS' if row['passed'] else 'FAIL'} / 指摘{len(items)}件", flush=True)
        report["status"] = "PASS" if len(report["runs"]) == len(entries) and all(r["passed"] for r in report["runs"]) else "FAIL"
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report["status"] = "FAIL"
        report["error"] = str(error)
        if report.get("configuration", {}).get("status") == "running":
            report["configuration"]["status"] = "FAIL"
    save(output, report)
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang-tidy", default="clang-tidy-18")
    parser.add_argument("--gcc-install-dir")
    parser.add_argument("--guide", choices=sorted(GUIDE_CHECKS), default="both")
    parser.add_argument("--id", action="append", help="省略時は通常版の全Verify")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    entries = prepare_verify.collect()
    if args.id:
        unknown = set(args.id) - {e["id"] for e in entries}
        if unknown:
            parser.error("存在しないVerify ID: " + ", ".join(sorted(unknown)))
        entries = [e for e in entries if e["id"] in args.id]
    report = run(entries, args.clang_tidy, args.output, args.gcc_install_dir, guide=args.guide)
    print(markdown(report))
    return int(report["status"] != "PASS")


if __name__ == "__main__":
    raise SystemExit(main())
