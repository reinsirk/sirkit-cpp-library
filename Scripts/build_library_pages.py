#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""現行ソース・オンライン判定・CI結果からcompetitive-verifierのページを生成する。"""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import hashlib
import html
from html.parser import HTMLParser
import importlib.metadata
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import sys
import tempfile
from urllib.parse import quote, unquote, urlsplit
from urllib.request import urlopen

sys.dont_write_bytecode = True
try:
    from . import ci_quality_report, update_verify_status
except ImportError:
    import ci_quality_report
    import update_verify_status

ROOT = Path(__file__).resolve().parents[1]
REPOSITORY = "reinsirk/sirkit-cpp-library"
VERIFIER_VERSION = "4.2.2"
VERIFIER_REVISION = "d6bfe076aa6ae891f93588d470c7886ad6208bd9"
# 原文はGit管理せず、固定commitから取得して生成サイトだけへ同梱する。
VERIFIER_LICENSES = (
    ("MIT-competitive-verifier.txt", "LICENSE",
     "13dc1487cea52dcd08c7f9781785f8bacac416e670d20e7fe3e07d67d80bf56c"),
    ("competitive-verifier-third-party-notices.txt", "3rd-party-license.txt",
     "d4c98d6e36d0a70b1ec89304693e3314c4063341a6d988c15a7f6288205617ce"),
)
SOURCE_DIRS = ("Graph", "DataStructure", "Math", "String", "Geometry", "Algorithm", "Internal")
CATEGORIES = {"normal": "正常系", "failure": "失敗系", "library_checker": "Library Checker公式ケース",
              "performance": "性能測定", "sanitizers": "ASan/UBSan", "coverage": "カバレッジ"}
LABELS = {"PASS": "合格", "FAIL": "失敗", "STALE": "ソース変更後は未検証", "UNAVAILABLE": "未実施",
          "PARTIAL": "一部のみ", "current_ac": "現行AC", "source_changed": "現行未照合・要再提出",
          "unsubmitted": "未提出", "non_ac": "非AC", "local_only": "ローカル補足"}
LINK = re.compile(r'(!?\[[^\]\n]*\])\((<[^>\n]+>|(?:[^\s()]|\([^()\s]*\))+)(?:\s+"[^"]*")?\)')


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dump(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def md(value: object) -> str:
    return update_verify_status.markdown(value).replace("{", "&#123;").replace("}", "&#125;")


def site_link(path: str) -> str:
    return "{{ '/" + quote(path, safe="/._-") + "' | relative_url }}"


def github_link(path: str, revision: str) -> str:
    return "https://github.com/" + REPOSITORY + "/blob/" + quote(revision, safe="") + "/" + quote(path, safe="/._-")


def literal_liquid(text: str) -> str:
    """C++の{{...}}等を、Jekyllの式ではなく元の文字として一度だけ出力する。"""
    return re.sub(r"\{\{|\{%", lambda match: "{{ '" + match[0] + "' }}", text)


def protect_reference_literals(text: str) -> str:
    """説明中の|を表と誤認させず、演算子の見出しを組版による変換から守る。"""
    lines, output, fence = text.splitlines(keepends=True), [], None
    for index, line in enumerate(lines):
        delimiter = re.match(r"^ {0,3}(`{3,}|~{3,})", line)
        if delimiter:
            token = delimiter[1]
            if fence is None:
                fence = token
            elif token[0] == fence[0] and len(token) >= len(fence):
                fence = None
            output.append(line)
            continue
        if fence is not None:
            output.append(line)
            continue
        heading = re.fullmatch(r"(#{2,6}) ([^`\n]+)\n?", line)
        if heading and re.fullmatch(r"[A-Za-z_][A-Za-z_0-9:<>\[\]()*~^|+./ =-]*", heading[2]):
            following = next((value.strip() for value in lines[index + 1:] if value.strip()), "")
            if following == "```text":
                line = heading[1] + " `" + heading[2] + "`\n"
        cursor, parts = 0, []
        for code in re.finditer(r"(`+)(.*?)\1", line):
            parts.extend([re.sub(r"(?<!\\)\|", r"\\|", line[cursor:code.start()]), code[0]])
            cursor = code.end()
        parts.append(re.sub(r"(?<!\\)\|", r"\\|", line[cursor:]))
        output.append("".join(parts))
    return "".join(output)


def explicit_page_links(value: object, pages: set[str]) -> None:
    """CVのリンク情報だけに.htmlを付け、通常の静的サーバーでも辿れるようにする。"""
    if isinstance(value, dict):
        if "icon" in value and value.get("path") in pages:
            value["path"] += ".html"
        for item in value.values():
            explicit_page_links(item, pages)
    elif isinstance(value, list):
        for item in value:
            explicit_page_links(item, pages)


def distinguish_page_titles(files: dict) -> None:
    """共通の説明・問題名を使う別ページは、対象ファイルも名前に含める。"""
    grouped = {}
    for path, data in files.items():
        title = data["document_attributes"]["TITLE"]
        grouped.setdefault(title, []).append(path)
    for title, paths in grouped.items():
        if len(paths) > 1:
            for path in paths:
                files[path]["document_attributes"]["TITLE"] = title + "（" + path + "）"
    titles = [data["document_attributes"]["TITLE"] for data in files.values()]
    if len(set(titles)) != len(titles):
        raise ValueError("ページタイトルが重複しています。page-titles.jsonを確認してください")


def validate_site(site: Path, pages: set[str], baseurl: str,
                  expected_apis: dict[str, str] | None = None) -> dict:
    class Links(HTMLParser):
        def __init__(self):
            super().__init__()
            self.urls = []
            self.api_blocks = []
            self.api_text = None

        def handle_starttag(self, tag, attributes):
            self.urls.extend(value for key, value in attributes if key in {"href", "src"} and value)
            if tag == "pre" and "api-reference-text" in dict(attributes).get("class", "").split():
                self.api_text = ""

        def handle_data(self, data):
            if self.api_text is not None:
                self.api_text += data

        def handle_endtag(self, tag):
            if tag == "pre" and self.api_text is not None:
                self.api_blocks.append(self.api_text)
                self.api_text = None

    missing = [path + ".html" for path in sorted(pages) if not (site / (path + ".html")).is_file()]
    missing += [path for path in ("index.html", "quality.html") if not (site / path).is_file()]
    broken, api_mismatches, count = [], [], 0
    for path in site.rglob("*.html"):
        count += 1
        links = Links()
        links.feed(path.read_text(encoding="utf-8"))
        source = path.relative_to(site).as_posix().removesuffix(".html")
        if expected_apis is not None and source in expected_apis:
            if links.api_blocks != [expected_apis[source]]:
                api_mismatches.append(source)
        for url in links.urls:
            parts = urlsplit(url)
            if parts.scheme or parts.netloc or not parts.path:
                continue
            target = unquote(parts.path)
            if baseurl and (target == baseurl or target.startswith(baseurl + "/")):
                target = target[len(baseurl):] or "/"
            resolved = site / target.lstrip("/") if target.startswith("/") else path.parent / target
            if resolved.is_dir():
                resolved /= "index.html"
            if not resolved.resolve().is_relative_to(site.resolve()) or not resolved.is_file():
                broken.append({"page": path.relative_to(site).as_posix(), "url": url})
    return {"html_pages": count, "missing_pages": missing, "broken_local_links": broken,
            "api_mismatches": api_mismatches, "api_pages_checked": len(expected_apis or {}),
            "passed": not missing and not broken and not api_mismatches}


def source_paths(root: Path) -> list[str]:
    result = []
    for folder in SOURCE_DIRS:
        for path in (root / folder).rglob("*.cpp"):
            if "components" not in path.relative_to(root).parts:
                if path.is_symlink() or not path.resolve().is_relative_to(root.resolve()):
                    raise ValueError("ライブラリのsymlinkは対象にできません: " + str(path))
                result.append(path.relative_to(root).as_posix())
    return sorted(result)


def document_path(root: Path, source: str) -> str | None:
    text = (root / source).read_text(encoding="utf-8")
    match = re.search(r"Document/([A-Za-z0-9_./-]+\.md)", text[:12000])
    if match:
        path = "Document/" + match[1]
        resolved = (root / path).resolve()
        if (resolved.is_relative_to(root.resolve()) and resolved.is_relative_to((root / "Document").resolve())
                and resolved.is_file() and not (root / path).is_symlink()):
            return path
    return None


def scoped_document_path(root: Path, source: str, overrides: dict[str, str]) -> str | None:
    if source not in overrides:
        return document_path(root, source)
    name = overrides[source]
    if not isinstance(name, str):
        raise ValueError("ページの説明文書はDocument内のパスを指定してください")
    path = root / name
    if (not name.startswith("Document/") or not name.endswith(".md") or path.is_symlink()
            or not path.resolve().is_relative_to(root.resolve())
            or not path.resolve().is_relative_to((root / "Document").resolve()) or not path.is_file()):
        raise ValueError("ページの説明文書が見つからないか、Document外を指しています: " + name)
    return name


def source_api_text(text: str) -> str:
    """先頭に集約されたAPIコメントだけを取得し、実装や書き換え用印は混ぜない。"""
    header = re.split(r"(?m)^#include\b", text, maxsplit=1)[0]
    marker = re.search(r"(?m)^// API一覧[^\n]*\n", header)
    if not marker:
        raise ValueError("ソース先頭にAPI一覧がありません")
    lines = []
    for line in header[marker.end():].splitlines():
        if line.startswith("// SIRKIT_NAMESPACE"):
            break
        if line.startswith("//"):
            lines.append(line[2:].removeprefix(" "))
        elif line.strip():
            break
        else:
            lines.append("")
    result = "\n".join(lines).strip()
    if not result:
        raise ValueError("ソース先頭のAPI一覧が空です")
    return result


def api_reference(text: str) -> str:
    # Liquidより前にエスケープし、テンプレート型・波括弧・式を元の文字として表示する。
    escaped = html.escape(text).replace("{", "&#123;").replace("}", "&#125;")
    return ('\n<details class="api-reference"><summary>ソースのAPI注記</summary>\n'
            '<pre class="api-reference-text">' + escaped + '</pre>\n</details>\n')


def rewrite_links(text: str, document: str, root: Path, sources: set[str], revision: str) -> str:
    def replace(match):
        label, destination = match.groups()
        destination = destination.removeprefix("<").removesuffix(">")
        if destination.startswith(("#", "https://", "http://", "mailto:", "{{")):
            return match[0]
        parts = urlsplit(destination)
        target = (root / document).parent / unquote(parts.path)
        resolved = target.resolve()
        if not resolved.is_relative_to(root.resolve()):
            return label + "(" + github_link(document, revision) + ")"
        relative = resolved.relative_to(root.resolve()).as_posix()
        url = site_link(relative + ".html") if relative in sources else github_link(relative, revision)
        if parts.fragment:
            url += "#" + quote(parts.fragment, safe="-_")
        return label + "(" + url + ")"
    lines, fence = [], None
    for line in text.splitlines(keepends=True):
        delimiter = re.match(r"^ {0,3}(`{3,}|~{3,})", line)
        if delimiter:
            value = delimiter[1]
            if fence is None:
                fence = value
            elif value[0] == fence[0] and len(value) >= len(fence):
                fence = None
            lines.append(line)
        elif fence is not None:
            lines.append(line)
        else:
            # 例のC++文字列やinline codeをリンクとして書き換えない。
            cursor, parts = 0, []
            for code in re.finditer(r"(`+)(.*?)\1", line):
                parts += [LINK.sub(replace, line[cursor:code.start()]), code[0]]
                cursor = code.end()
            parts.append(LINK.sub(replace, line[cursor:]))
            lines.append("".join(parts))
    return "".join(lines)


def online_table(entries: list[dict]) -> str:
    lines = ["### オンラインVerify", "", "| 問題 | 現行ソースの状態 | 提出結果 |", "| --- | --- | --- |"]
    if not entries:
        return "### オンラインVerify\n\n対応するVerifyは未登録です。\n"
    for entry in entries:
        problem = md(entry["title"])
        if entry.get("problem_url"):
            problem = "[" + problem + "](" + update_verify_status.safe_url(entry["problem_url"]) + ")"
        record = entry.get("judgement", {})
        result = "—"
        if record.get("url"):
            result = "[" + md(record.get("status", "記録")) + "](" + update_verify_status.safe_url(record["url"]) + ")"
        policy = entry.get("submission_policy", {}).get("kind")
        condition = ("説明変更のみ・再提出不要" if policy == "documentation_only" else
                     "参考検証・移行先あり" if policy == "reference" else LABELS[entry["status"]])
        lines.append("| " + problem + " | " + condition + " | " + result + " |")
    lines += ["", "過去のACは、現行の提出全文と一致する場合だけ「現行AC」と表示します。"]
    return "\n".join(lines) + "\n"


def performance_table(evidence: list[dict]) -> str:
    rows = [item for item in evidence if "median_ns" in item]
    if not rows:
        return ""
    def safe(value):
        return ci_quality_report.inline(value).replace("{", "&#123;").replace("}", "&#125;")
    lines = ["起動・入出力込みのVerify全体と、本体だけの測定を区別します。時間は中央値です。", "",
             "| 測定範囲 | ケース | 規模 | wall ms | CPU ms | 状態 |",
             "| --- | --- | --- | ---: | ---: | --- |"]
    for item in rows:
        workload = item.get("workload", {})
        scale = workload.get("dimensions") or workload.get("input_header") or str(item["size"]) + (" bytes" if item.get("size_unit") == "input_bytes" else "要素")
        scope = item["scope"] + ("・縮小確認のみ" if item.get("size_profile") == "quick_smoke" or item.get("scale_scope") == "smoke_only" else "")
        cpu = item.get("cpu_median_ns", {}).get("current")
        cpu_text = f"{cpu/1e6:.3f}" if cpu is not None else "対象外"
        lines.append(f"| {safe(scope)} | {safe(item['pattern'])} | {safe(scale)} | {item['median_ns']['current']/1e6:.3f} | {cpu_text} | {LABELS[item['status']]} |")
    environments = set()
    for item in rows:
        compiler = item.get("compiler", {})
        environment = (compiler.get("version", "記録なし") if isinstance(compiler, dict) else compiler,
                       " ".join(item.get("flags", [])), item.get("cpu_model") or "記録なし",
                       item.get("repetitions"), item.get("warmups"))
        if environment not in environments:
            environments.add(environment)
            lines += ["", "測定環境: " + safe(environment[0]) + " / " + safe(environment[1]) + " / " + safe(environment[2])
                      + f"。測定{environment[3]}回、ウォームアップ{environment[4]}回。"]
    return "\n".join(lines) + "\n"


def quality_table(library: dict | None) -> str:
    categories = (library or {}).get("categories", {})
    lines = ["### ローカル・CI検査", "", "| 検査 | 状態 |", "| --- | --- |"]
    for key, label in CATEGORIES.items():
        status = categories.get(key, {}).get("status", "UNAVAILABLE")
        lines.append("| " + label + " | " + LABELS.get(status, "未実施") + " |")
    lines += ["", "[実行条件・件数・測定結果](" + site_link("quality.html") + ") · "
              "[結果JSON](" + site_link("assets/quality.json") + ")"]
    for key in ("failure", "performance", "library_checker", "coverage"):
        evidence = categories.get(key, {}).get("evidence", [])
        if not evidence:
            continue
        lines += ["", '<details markdown="1"><summary>' + CATEGORIES[key] + "の詳細</summary>", ""]
        if key == "performance":
            lines += [performance_table(evidence), "<details><summary>測定の生データ</summary>", ""]
        for item in evidence:
            # 表示内容もJSON由来。生のHTML・コマンドを解釈しない。
            encoded = html.escape(json.dumps(item, ensure_ascii=False, indent=2))
            encoded = encoded.replace("{", "&#123;").replace("}", "&#125;")
            lines += ["<pre><code class=\"language-json\">" + encoded + "</code></pre>", ""]
        if key == "performance":
            lines += ["</details>"]
        lines += ["</details>"]
    return "\n".join(lines) + "\n"


@contextmanager
def isolated_index(root: Path, paths: list[str]):
    """通常index・object DBを変更せず、新規ソースと生成Markdownも収集する。"""
    with tempfile.TemporaryDirectory(prefix="sirkit-pages-index-") as directory:
        temporary = Path(directory)
        objects = temporary / "objects"
        objects.mkdir()
        original = subprocess.check_output(["git", "rev-parse", "--git-path", "objects"], cwd=root, text=True).strip()
        environment = dict(os.environ)
        environment.update(GIT_INDEX_FILE=str(temporary / "index"), GIT_OBJECT_DIRECTORY=str(objects),
                           GIT_ALTERNATE_OBJECT_DIRECTORIES=str((root / original).resolve()))
        subprocess.run(["git", "read-tree", "--empty"], cwd=root, env=environment, check=True)
        names = temporary / "paths"
        names.write_bytes(b"".join(path.encode("utf-8") + b"\0" for path in paths))
        subprocess.run(["git", "-c", "core.hooksPath=/dev/null", "add", "-f", "--pathspec-from-file=" + str(names),
                        "--pathspec-file-nul"], cwd=root, env=environment, check=True)
        yield environment


def upstream_license_sources() -> dict:
    return {"version": VERIFIER_VERSION, "revision": VERIFIER_REVISION, "files": [
        {"filename": name, "url": "https://raw.githubusercontent.com/competitive-verifier/"
         "competitive-verifier/" + VERIFIER_REVISION + "/" + path, "sha256": digest}
        for name, path, digest in VERIFIER_LICENSES]}


def fetch_upstream_licenses() -> dict[str, bytes]:
    result = {}
    for entry in upstream_license_sources()["files"]:
        with urlopen(entry["url"], timeout=30) as response:
            data = response.read(1024 * 1024 + 1)
        if hashlib.sha256(data).hexdigest() != entry["sha256"]:
            raise ValueError("固定版ライセンスのSHA-256が一致しません: " + entry["filename"])
        result[entry["filename"]] = data
    return result


def prepare_output(root: Path, output: Path) -> None:
    output = output.resolve()
    if output == root or root.is_relative_to(output):
        raise ValueError("出力先にリポジトリやその親は指定できません")
    if output.is_relative_to(root) and not output.is_relative_to(root / ".competitive-verifier/build"):
        raise ValueError("リポジトリ内の出力先は.competitive-verifier/build以下にしてください")
    marker = output / ".sirkit-pages-output"
    if output.exists() and any(output.iterdir()) and not marker.is_file():
        raise ValueError("既存データのある出力先は使えません: " + str(output))
    output.mkdir(parents=True, exist_ok=True)
    marker.write_text("sirkit library pages\n")
    for name in ("jekyll", "site", "reports", "docs"):
        path = output / name
        if path.is_symlink():
            raise ValueError("出力先のsymlinkは使用できません")
        if path.exists():
            shutil.rmtree(path)


def build(root: Path, output: Path, reports: list[Path], *, production: bool = False,
          baseurl: str = "/sirkit-cpp-library", render: bool = False) -> dict:
    root, output = root.resolve(), output.resolve()
    if not re.fullmatch(r"(?:/[A-Za-z0-9_.-]+)*", baseurl):
        raise ValueError("baseurlは空文字か安全なパスを指定してください")
    try:
        version = importlib.metadata.version("competitive-verifier")
    except importlib.metadata.PackageNotFoundError as error:
        raise ValueError(".competitive-verifier/requirements.txtをインストールしてください") from error
    if version != VERIFIER_VERSION:
        raise ValueError("competitive-verifierは固定版" + VERIFIER_VERSION + "を使用してください: " + version)
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, text=True).strip()
    dirty = bool(subprocess.check_output(["git", "status", "--porcelain", "--untracked-files=normal"], cwd=root))
    if production and dirty:
        raise ValueError("公開版は未コミットの変更を含まないcheckoutで生成してください")
    # 通信失敗・内容不一致のときは既存のプレビューも上書きしない。
    license_files = fetch_upstream_licenses()
    prepare_output(root, output)
    sources = source_paths(root)
    source_set = set(sources)
    titles_path = root / ".competitive-verifier/page-titles.json"
    title_overrides = json.loads(titles_path.read_text(encoding="utf-8"))
    if (not isinstance(title_overrides, dict) or set(title_overrides) - source_set
            or any(not isinstance(title, str) or not title.strip() for title in title_overrides.values())):
        raise ValueError("page-titles.jsonは存在する通常版ファイルと空でないタイトルを指定してください")
    documents_path = root / ".competitive-verifier/page-documents.json"
    document_overrides = json.loads(documents_path.read_text(encoding="utf-8"))
    if not isinstance(document_overrides, dict) or set(document_overrides) - source_set:
        raise ValueError("page-documents.jsonは存在する通常版ファイルを指定してください")
    watched_paths = set(sources)
    watched_paths.add(titles_path.relative_to(root).as_posix())
    watched_paths.add(documents_path.relative_to(root).as_posix())
    for pattern in ("Document/**/*.md", "Verify/current/drivers/*", "Verify/current/common/*",
                    "Verify/current/groups/*.json", "Verify/current/submissions/*.cpp"):
        for path in root.glob(pattern):
            if path.is_symlink() or not path.resolve().is_relative_to(root):
                raise ValueError("ページの入力にsymlinkは使用できません: " + str(path))
            if path.is_file():
                watched_paths.add(path.relative_to(root).as_posix())
    watched_paths.update(["Verify/current/manifest.json", "Verify/current/judgements.json"])
    watched = {name: sha(root / name) for name in sorted(watched_paths)}
    status = update_verify_status.build_status(root)
    quality = ci_quality_report.aggregate(root, reports)
    for entry in status["entries"]:
        for name, recorded in entry["source_hashes"].items():
            if recorded != watched.get(name):
                raise ValueError("判定集計中にソースが変更されました: " + name)
    for library in quality["libraries"]:
        if library["path"] in watched and library["source_sha256"] != watched[library["path"]]:
            raise ValueError("テスト集計中にソースが変更されました: " + library["path"])
    dump(output / "reports/quality.json", quality)
    (output / "reports/quality.md").write_text(ci_quality_report.markdown(quality), encoding="utf-8")
    dump(output / "reports/online-status.json", status)
    by_library = {item["path"]: item for item in quality["libraries"]}
    snapshot = {"format": 1, "revision": revision, "dirty": dirty, "production": production,
                "source_hashes": {path: watched[path] for path in sources},
                "online_status": "computed_from_current_submission_sha256", "competitive_verifier": version}
    dump(output / "reports/snapshot.json", snapshot)
    generated = root / ".competitive-verifier/generated"
    generated.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="pages-", dir=generated) as folder:
        wrappers = Path(folder)
        files, results, paths, page_content, page_apis = {}, {}, [], {}, {}
        for source in sources:
            source_text = (root / source).read_text(encoding="utf-8")
            api = source_api_text(source_text)
            page_apis[source] = api
            document = scoped_document_path(root, source, document_overrides)
            content = (root / document).read_text(encoding="utf-8") if document else ""
            title = content.splitlines()[0].removeprefix("# ") if content.startswith("# ") else Path(source).stem
            title = title_overrides.get(source, title)
            if content.startswith("# "):
                content = "\n".join(content.splitlines()[1:]).lstrip()
            if document:
                content = protect_reference_literals(content)
                content = literal_liquid(content)
                content = rewrite_links(content, document, root, source_set, revision)
            related = [entry for entry in status["entries"] if source in entry["libraries"]]
            body = content or "ソース先頭のAPI一覧を参照してください。\n"
            body += api_reference(api)
            body += ('\n<details class="verification-details" markdown="1"><summary>検証結果</summary>\n\n'
                     + online_table(related) + "\n" + quality_table(by_library.get(source)) + '\n</details>\n')
            page_content[source] = {"document": document, "source_sha256": watched[source],
                                    "api_sha256": hashlib.sha256(api.encode("utf-8")).hexdigest()}
            path = wrappers / (source + ".md")
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("---\ndocumentation_of: " + json.dumps("//" + source) + "\n---\n\n" + body, encoding="utf-8")
            attrs = {"TITLE": title, "SIRKIT_SOURCE_SHA256": sha(root / source)}
            if document:
                attrs["SIRKIT_DOCUMENT"] = document
            files[source] = {"document_attributes": attrs}
            paths += [source, path.relative_to(root).as_posix()]
        for entry in status["entries"]:
            path = entry["submission"]
            if not (root / path).is_file():
                continue
            files[path] = {"dependencies": [name for name in entry["libraries"] if name in source_set],
                           "verification": {"type": "const", "status": "skipped"},
                           "document_attributes": {"TITLE": entry["title"], "DISPLAY": "no-index"}}
            results[path] = {"verifications": []}
            paths.append(path)
        distinguish_page_titles(files)
        dump(output / "reports/page-titles.json", {
            path: data["document_attributes"]["TITLE"] for path, data in files.items()})
        dump(output / "reports/page-content.json", page_content)
        dump(output / "verification-input.json", {"files": files})
        # CV自身の検証を実行したことにはしない。実証された結果は本文の別表で表示する。
        dump(output / "verification-result.json", {"total_seconds": 0, "files": results})
        docs_source = root / ".competitive-verifier/docs"
        if (not docs_source.resolve().is_relative_to(root) or docs_source.is_symlink()
                or any(path.is_symlink() for path in docs_source.rglob("*"))):
            raise ValueError("ページの設定・静的ファイルにsymlinkは使用できません")
        docs = output / "docs"
        shutil.copytree(docs_source, docs, dirs_exist_ok=True)
        import yaml
        config = yaml.safe_load((docs / "_config.yml").read_text(encoding="utf-8"))
        config.update(baseurl=baseurl, branch_name=revision, sirkit_preview=not production,
                      url=config.get("url", "") if production else "")
        # 固定の公開情報は明示し、ローカルHTML生成時のGitHub APIアクセスを不要にする。
        config["github"] = {"repository_url": "https://github.com/" + REPOSITORY,
                            "repository_nwo": REPOSITORY, "repository_name": REPOSITORY.split("/")[1],
                            "project_tagline": config["description"],
                            "owner_name": REPOSITORY.split("/")[0], "owner_url": "https://github.com/" + REPOSITORY.split("/")[0],
                            "is_project_page": True, "is_user_page": False, "build_revision": revision}
        with isolated_index(root, paths) as environment:
            environment["COMPETITIVE_VERIFY_DOCS_CONFIG_YML"] = json.dumps(config, ensure_ascii=False)
            environment["COMPETITIVE_VERIFY_CONFIG_PATH"] = str(output / "cv-cache")
            subprocess.run([sys.executable, "-c", "from competitive_verifier.app import main; raise SystemExit(main())", "docs", "--verify-json",
                            str(output / "verification-input.json"), str(output / "verification-result.json"),
                            "--docs", str(docs), "--destination", str(output / "jekyll")],
                           cwd=root, env=environment, check=True)
    # CVの標準リンクは拡張子なし。GitHub以外のローカルサーバーにも対応する。
    for path in (output / "jekyll").rglob("*.md"):
        text = path.read_text(encoding="utf-8")
        if text.startswith("---\n"):
            front, body = text[4:].split("\n---", 1)
            data = yaml.safe_load(front)
            explicit_page_links(data, set(files))
            path.write_text("---\n" + yaml.safe_dump(data, allow_unicode=True, sort_keys=False) + "---" + body, encoding="utf-8")
    # ドット付き一時ディレクトリ内のplain MarkdownはJekyllが除外するため、補助ページはルートへ置く。
    (output / "jekyll/quality.md").write_text(
        "---\nlayout: default\npermalink: /quality.html\ntitle: テスト・性能\n---\n\n"
        "[ライブラリ一覧](" + site_link("") + ")\n\n# テスト・性能\n\n" +
        ci_quality_report.markdown(quality) + "\n[結果JSON](" + site_link("assets/quality.json") + ")\n", encoding="utf-8")
    assets = output / "jekyll/assets"
    assets.mkdir(parents=True, exist_ok=True)
    for src, dst in (("quality.json", "quality.json"), ("online-status.json", "online-status.json"), ("snapshot.json", "snapshot.json")):
        shutil.copyfile(output / "reports" / src, assets / dst)
    licenses = output / "jekyll/licenses"
    licenses.mkdir(parents=True, exist_ok=True)
    for name, data in license_files.items():
        (licenses / name).write_bytes(data)
    dump(licenses / "sources.json", upstream_license_sources())
    # GitHub APIへの接続を要するmetadata pluginは使わず、表示に必要なgemだけを固定する。
    shutil.copyfile(root / ".competitive-verifier/Gemfile", output / "jekyll/Gemfile")
    lockfile = root / ".competitive-verifier/Gemfile.lock"
    if lockfile.is_file():
        shutil.copyfile(lockfile, output / "jekyll/Gemfile.lock")
    changed = [name for name, recorded in watched.items() if not (root / name).is_file() or sha(root / name) != recorded]
    latest_revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, text=True).strip()
    if changed or latest_revision != revision:
        raise ValueError("ページ生成中に入力が変更されました。再生成してください: " + ", ".join(changed[:10]))
    if render:
        subprocess.run(["bundle", "config", "set", "--local", "path", str(output / "bundle")], cwd=output / "jekyll", check=True)
        subprocess.run(["bundle", "install", "--jobs", "4", "--retry", "2"], cwd=output / "jekyll", check=True)
        subprocess.run(["bundle", "exec", "jekyll", "build", "--destination", str(output / "site")], cwd=output / "jekyll", check=True)
        validation = validate_site(output / "site", set(files), baseurl, page_apis)
        dump(output / "reports/html-validation.json", validation)
        if not validation["passed"]:
            raise ValueError("生成HTMLのページ・リンクが不整合です。reports/html-validation.jsonを確認してください")
    result = {**snapshot, "library_pages": len(sources), "verify_pages": len(results),
              "jekyll": str(output / "jekyll"), "site": str(output / "site") if render else None,
              "quality_status": quality["status"], "published": False}
    dump(output / "build-result.json", result)
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--report", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--production", action="store_true")
    parser.add_argument("--baseurl", default="/sirkit-cpp-library")
    parser.add_argument("--render", action="store_true")
    args = parser.parse_args(argv)
    try:
        result = build(args.root, args.output, args.report, production=args.production,
                       baseurl=args.baseurl, render=args.render)
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return 0
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
