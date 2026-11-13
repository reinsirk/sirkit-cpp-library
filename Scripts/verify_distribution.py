#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Independently inspect a full or flattened sirkit ZIP against manifests read now."""
from __future__ import annotations

import argparse
import ast
import datetime
import hashlib
import json
import pathlib
import re
import stat
import zipfile
from urllib.parse import unquote, urlsplit

EDITIONS = {"full": None, "gcc": ("GCC", False), "clang": ("Clang", False),
            "fast_gcc": ("GCC", True), "fast_clang": ("Clang", True)}
VERIFY_LIMIT = 512 * 1024


def digest(data):
    return hashlib.sha256(data).hexdigest()


def canonical_path(name, *, directory=False):
    """Strict portable ZIP names; no extraction is performed."""
    if not isinstance(name, str) or not name or any(ord(c) < 32 for c in name):
        raise ValueError("empty/control path")
    if name.startswith("/") or "\\" in name or ":" in name:
        raise ValueError("absolute/Windows path")
    value = name[:-1] if directory and name.endswith("/") else name
    if not value or any(part in ("", ".", "..") for part in value.split("/")):
        raise ValueError("noncanonical path")
    return value


def relative_target(owner, target):
    if not target or target.startswith("/") or any(c in target for c in "\\:\x00"):
        raise ValueError("unsafe relative target")
    parts = owner.split("/")[:-1]
    for part in target.split("/"):
        if part in ("", "."):
            continue
        if part == "..":
            if not parts:
                raise ValueError("target escapes archive")
            parts.pop()
        else:
            parts.append(part)
    return "/".join(parts)


def lexical_regions(text):
    """Return comments/literals, retaining exact source offsets and newlines."""
    regions = []
    i = 0
    while i < len(text):
        start = i
        if text.startswith("//", i):
            i = text.find("\n", i + 2)
            if i < 0:
                i = len(text)
            while i < len(text) and text[start:i].rstrip("\r").endswith("\\"):
                following = text.find("\n", i + 1)
                i = len(text) if following < 0 else following
            regions.append((start, i, "comment"))
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = len(text) if end < 0 else end + 2
            regions.append((start, i, "comment"))
        elif text.startswith('R"', i):
            opening = text.find("(", i + 2, i + 19)
            delimiter = text[i + 2:opening] if opening >= 0 else None
            if delimiter is not None and not any(c.isspace() or c in "()\\" for c in delimiter):
                end = text.find(")" + delimiter + '"', opening + 1)
                i = len(text) if end < 0 else end + len(delimiter) + 2
                regions.append((start, i, "literal"))
            else:
                i += 1
        elif text[i] in "\"'":
            # Apostrophes inside pp-numbers are digit separators.
            if text[i] == "'" and i and (text[i - 1].isalnum() or text[i - 1] in "_.'"):
                previous = i - 1
                while previous and (text[previous - 1].isalnum() or text[previous - 1] in "_.'"):
                    previous -= 1
                if text[previous].isdigit() or text[previous] == ".":
                    i += 1
                    continue
            quote = text[i]
            i += 1
            while i < len(text):
                if text[i] == "\\":
                    i += 2
                elif text[i] == quote:
                    i += 1
                    break
                else:
                    i += 1
            i = min(i, len(text))
            regions.append((start, i, "literal"))
        else:
            i += 1
    return regions


def code_mask(text):
    mask = list(text)
    for begin, end, _kind in lexical_regions(text):
        for i in range(begin, end):
            if mask[i] not in "\r\n":
                mask[i] = " "
    return "".join(mask)


def includes(text):
    """Active lexical directives, including directives in conditional branches."""
    # Phase-2 splicing is done only for scanning. Offsets map back to physical text.
    logical, offsets = [], []
    i = 0
    while i < len(text):
        if text.startswith("\\\r\n", i):
            i += 3
        elif text.startswith("\\\n", i):
            i += 2
        else:
            logical.append(text[i]); offsets.append(i); i += 1
    source = "".join(logical)
    mask = code_mask(source)
    result = []
    for match in re.finditer(r"(?m)^[ \t\r]*#[ \t]*include\b", mask):
        hash_at = mask.index("#", match.start(), match.end())
        tail = source[match.end():]
        literal = re.match(r'[ \t]*(?:"([^"\r\n]+)"|<([^>\r\n]+)>)', tail)
        if literal:
            end = match.end() + literal.end()
            result.append(dict(begin=offsets[hash_at], end=offsets[end - 1] + 1,
                               name=literal.group(1) or literal.group(2), quoted=literal.group(1) is not None))
        else:
            result.append(dict(begin=offsets[hash_at], end=offsets[match.end() - 1] + 1,
                               name=None, quoted=False))
    return result


def remove_unprotected_blank_lines(text):
    """The ACL formatter may change empty lines; literal contents remain exact."""
    protected = bytearray(len(text))
    for begin, end, kind in lexical_regions(text):
        if kind == "literal":
            protected[begin:end] = b"\1" * (end - begin)
    out, position = [], 0
    for line in text.splitlines(keepends=True):
        end = position + len(line)
        if line.strip() or any(protected[position:end]):
            out.append(line)
        position = end
    return "".join(out)


def expand_reference(text, owner, known, loader, acl_loader, *, limit=32 * 1024 * 1024):
    """Independent recursive expansion; no production helper is imported."""
    def own(source, current, stack):
        out, offset = [], 0
        for directive in includes(source):
            if not directive["quoted"] or directive["name"] is None:
                continue
            name = directive["name"]
            if name.startswith("atcoder/"):
                continue
            target = relative_target(current, name)
            if target not in known:
                continue
            if target in stack:
                raise ValueError("cyclic own include")
            out.extend((source[offset:directive["begin"]], own(loader(target), target, stack + (target,))))
            offset = directive["end"]
        out.append(source[offset:])
        result = "".join(out)
        if len(result) > limit:
            raise ValueError("reference expansion budget")
        return result

    seen = set()
    def acl(source):
        out, offset = [], 0
        for directive in includes(source):
            name = directive["name"]
            if name is None or not name.startswith("atcoder/"):
                continue
            key = name.removesuffix(".hpp")
            if not re.fullmatch(r"atcoder/[a-z_]+", key):
                raise ValueError("unsafe ACL include")
            body = "" if key in seen else "// ACL: " + key + "\n"
            if key not in seen:
                seen.add(key)
                body += acl(acl_loader(key + ".hpp"))
            out.extend((source[offset:directive["begin"]], body)); offset = directive["end"]
        out.append(source[offset:])
        result = "".join(out)
        if len(result) > limit:
            raise ValueError("ACL expansion budget")
        return result
    return acl(own(text, owner, ()))


def markdown_links(text):
    """Inline and reference-style Markdown links, with balanced destination ()."""
    usable = []
    fence = None
    for line in text.splitlines():
        marker = re.match(r"^ {0,3}(`{3,}|~{3,})", line)
        if marker:
            if fence is None:
                fence = marker.group(1)
            elif marker.group(1)[0] == fence[0] and len(marker.group(1)) >= len(fence):
                fence = None
            usable.append(""); continue
        usable.append(line if fence is None else "")
    source = "\n".join(usable)
    # Mask inline code without interpreting its apparent links.
    source = re.sub(r"(`+)([\s\S]*?)\1", lambda m: " " * len(m.group()), source)
    found = []
    for match in re.finditer(r"!?\[[^\]\n]*\]\(", source):
        start, depth = match.end(), 1
        i = start
        while i < len(source) and depth:
            if source[i] == "\\":
                i += 2; continue
            if source[i] == "(": depth += 1
            if source[i] == ")": depth -= 1
            if depth: i += 1
        if not depth:
            destination = source[start:i].strip()
            if destination.startswith("<") and ">" in destination:
                destination = destination[1:destination.index(">")]
            else:
                destination = re.split(r"\s+[\"']", destination, maxsplit=1)[0]
            found.append(destination)
    for match in re.finditer(r"(?m)^ {0,3}\[[^\]]+\]:\s*(?:<([^>]+)>|(\S+))", source):
        found.append(match.group(1) or match.group(2))
    return found


def node_example_adapter(text):
    mask = code_mask(text)
    spans = [m.span() for m in re.finditer(r"\bentry\s*\.\s*get\s*\(\s*\)", mask)]
    for begin, end in reversed(spans):
        text = text[:begin] + "entry" + text[end:]
    return text.replace("所有は残る", "参照は残る").replace("共有所有の循環を切ってから解放する", "循環を切る。メモリは解放しない")


def example_map(root):
    tree = ast.parse((root / "Scripts/package_revised_libraries.py").read_text())
    for statement in tree.body:
        if isinstance(statement, ast.Assign) and any(isinstance(t, ast.Name) and t.id == "LIBRARIES" for t in statement.targets):
            result = ast.literal_eval(statement.value)
            if not isinstance(result, dict):
                raise ValueError("LIBRARIES is not a literal mapping")
            return result
    raise ValueError("LIBRARIES mapping not found")


def validate_archive(root, path, edition, *, max_total=2 * 1024**3, max_entry=128 * 1024**2):
    root, path = pathlib.Path(root), pathlib.Path(path)
    report = dict(archive=str(path.resolve()), edition=edition, errors=[], warnings=[], stats={}, checked_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
    def issue(code, name, detail):
        report["errors"].append(dict(code=code, path=name, detail=str(detail)))
    if edition not in EDITIONS:
        raise ValueError("unknown edition")
    current = {}
    for name in ("Variants/manifest.json", "Variants/verify_manifest.json", "Verify/current/manifest.json", "Scripts/library_layout.json"):
        data = (root / name).read_bytes(); current[name] = json.loads(data)
    report["current_manifest_hashes"] = {name: digest((root / name).read_bytes()) for name in current}
    report["validator_sha256"] = digest(pathlib.Path(__file__).read_bytes())
    report["archive_sha256"] = digest(path.read_bytes())
    try:
        archive = zipfile.ZipFile(path)
    except (zipfile.BadZipFile, OSError) as error:
        issue("zip_open", "", error); report["ok"] = False; return report
    with archive:
        infos = archive.infolist(); names = set(); files = {}; folded = {}; total = 0
        for info in infos:
            name = info.filename
            try:
                canonical_path(info.orig_filename, directory=info.is_dir())
                if info.orig_filename != name:
                    raise ValueError("filename truncation")
            except ValueError as error:
                issue("unsafe_entry", name, error); continue
            if name in names:
                issue("duplicate_entry", name, "duplicate ZIP central-directory name")
            names.add(name)
            if name.casefold() in folded and folded[name.casefold()] != name:
                issue("casefold_collision", name, folded[name.casefold()])
            folded[name.casefold()] = name
            if stat.S_IFMT(info.external_attr >> 16) == stat.S_IFLNK:
                issue("symlink_entry", name, "symlink entries are not accepted")
            total += info.file_size
            if info.file_size > max_entry:
                issue("entry_budget", name, info.file_size)
            if not info.is_dir():
                files[name] = info
        for name in files:
            for parent in pathlib.PurePosixPath(name).parents:
                if str(parent) in files:
                    issue("file_directory_collision", name, str(parent))
        if total > max_total:
            issue("archive_budget", "", total)
        report["stats"].update(entries=len(infos), files=len(files), total_uncompressed_bytes=total)
        if any(e["code"] in {"unsafe_entry", "entry_budget", "archive_budget"} for e in report["errors"]):
            report["ok"] = False; return report
        crc_count = 0
        for info in infos:
            try:
                with archive.open(info) as stream:
                    while stream.read(1024 * 1024):
                        pass
                crc_count += 1
            except (zipfile.BadZipFile, RuntimeError, OSError) as error:
                issue("crc_or_read", info.filename, error)
        report["stats"]["crc_entries_checked"] = crc_count
        if any(e["code"] == "crc_or_read" for e in report["errors"]):
            report["ok"] = False; return report
        def read(name):
            if name not in files:
                raise KeyError(name)
            return archive.read(files[name])
        def expected(name, data, kind="content_sha"):
            if name not in files:
                issue("missing_entry", name, kind); return
            try:
                actual = read(name)
            except (zipfile.BadZipFile, RuntimeError, OSError) as error:
                issue("crc_or_read", name, error); return
            if digest(actual) != digest(data):
                issue(kind, name, dict(actual=digest(actual), expected=digest(data)))
        variants = current["Variants/manifest.json"]["entries"]
        verifies = current["Variants/verify_manifest.json"]["entries"]
        normal = current["Verify/current/manifest.json"]["entries"]
        selected = variants if edition == "full" else [e for e in variants if (e["compiler"], e["fast"]) == EDITIONS[edition]]
        selected_verify = verifies if edition == "full" else [e for e in verifies if (e["compiler"], e["fast"]) == EDITIONS[edition]]
        for entry in selected:
            data = (root / entry["path"]).read_bytes()
            if digest(data) != entry["sha256"]:
                issue("current_manifest_source_sha", entry["path"], "workspace source and current variant manifest differ")
            target = entry["path"] if edition == "full" else entry["source"]
            expected(target, data, "edition_source_sha")
        for entry in selected_verify:
            for origin, sha in entry.get("source_hashes", {}).items():
                source_path = entry.get("source_paths", {}).get(origin, origin)
                if digest((root / source_path).read_bytes()) != sha:
                    issue("current_verify_library_sha", source_path, entry["id"])
            if entry.get("driver_sha256") and digest((root / entry["driver"]).read_bytes()) != entry["driver_sha256"]:
                issue("current_verify_driver_sha", entry["driver"], entry["id"])
            for support, sha in entry.get("support_source_hashes", {}).items():
                if digest((root / support).read_bytes()) != sha:
                    issue("current_verify_support_sha", support, entry["id"])
            data = (root / entry["path"]).read_bytes()
            if digest(data) != entry["sha256"]:
                issue("current_manifest_verify_sha", entry["path"], "workspace submission and current manifest differ")
            target = entry["path"] if edition == "full" else "Verify/current/submissions/" + entry["id"] + ".cpp"
            expected(target, data, "verify_sha")
            if edition.startswith("fast_") and "DataStructure/Node.cpp" in entry.get("source_paths", {}) and target in files:
                code = code_mask(read(target).decode())
                main_at = re.search(r"\bint\s+main\s*\(", code)
                body = code[main_at.start():] if main_at else code
                if re.search(r"std\s*::\s*shared_ptr\s*<\s*Node\s*>|\b(?:entry|middle|head)\s*\.\s*get\s*\(|->\s*n\s*\.\s*reset\s*\(", body):
                    issue("node_driver_adapter", target, "shared_ptr API remains in Fast Node main")
        if edition == "full":
            source_names = {e["source"] for e in variants}
            for folder in current["Scripts/library_layout.json"]:
                source_names.update(p.relative_to(root).as_posix() for p in (root / folder).rglob("*.cpp"))
            for name in source_names:
                expected(name, (root / name).read_bytes(), "normal_source_sha")
            for entry in normal:
                for origin, sha in entry.get("source_hashes", {}).items():
                    if digest((root / origin).read_bytes()) != sha:
                        issue("current_normal_library_sha", origin, entry["id"])
                if entry.get("driver_sha256") and digest((root / entry["driver"]).read_bytes()) != entry["driver_sha256"]:
                    issue("current_normal_driver_sha", entry["driver"], entry["id"])
                for support, sha in entry.get("support_source_hashes", {}).items():
                    if digest((root / support).read_bytes()) != sha:
                        issue("current_normal_support_sha", support, entry["id"])
                name = entry["submission"]; data = (root / name).read_bytes()
                if digest(data) != entry["submission_sha256"]:
                    issue("current_normal_verify_sha", name, "workspace submission and current manifest differ")
                expected(name, data, "normal_verify_sha")
            for name in current:
                expected(name, (root / name).read_bytes(), "manifest_bytes")
            for name in ("Readme.md", "AGENTS.md", "temp.cpp", "temp2.cpp", "LICENSE"):
                if (root / name).is_file(): expected(name, (root / name).read_bytes())
        else:
            name = "Verify/current/manifest.json"
            if name not in files:
                issue("missing_entry", name, "flattened manifest")
            else:
                try:
                    actual_entries = json.loads(read(name))["entries"]
                    if not isinstance(actual_entries, list) or any(not isinstance(e, dict) or "id" not in e for e in actual_entries):
                        raise ValueError("invalid entries shape")
                except (ValueError, TypeError, KeyError) as error:
                    issue("manifest_parse", name, error); report["ok"] = False; return report
                by_id = {e["id"]: e for e in actual_entries}
                if len(by_id) != len(actual_entries) or set(by_id) != {e["id"] for e in selected_verify}:
                    issue("manifest_verify_ids", name, "selected edition IDs differ")
                for original in selected_verify:
                    record = by_id.get(original["id"])
                    if record is None: continue
                    want = dict(original); want["path"] = "Verify/current/submissions/" + original["id"] + ".cpp"
                    # Allow the minimal source_paths flattening repair; preserve provenance in other fields.
                    flattened = dict(want, source_paths={origin: origin for origin in want.get("source_paths", {})})
                    alternatives = [want, flattened, dict(flattened, upstream_source_paths=want.get("source_paths", {}))]
                    if record not in alternatives: issue("manifest_record", original["id"], "manifest record differs from current chosen edition")
                    for origin, sha in record.get("source_hashes", {}).items():
                        if origin not in files or digest(read(origin)) != sha:
                            issue("manifest_library_sha", origin, original["id"])
                    for origin, source_path in record.get("source_paths", {}).items():
                        if source_path not in files:
                            report["warnings"].append(dict(code="upstream_manifest_source_path", path=source_path, detail=original["id"]))
        examples = example_map(root)
        known = {e["source"] for e in variants}
        for example, library in examples.items():
            owner = "Examples/" + example
            source = (root / owner).read_text()
            if edition.startswith("fast_") and library == "DataStructure/Node.cpp":
                source = node_example_adapter(source)
            expected(owner, source.encode(), "example_edition_adapter")
            standalone = "StandaloneExamples/" + example
            if standalone not in files:
                issue("missing_entry", standalone, "standalone example"); continue
            try:
                reference = expand_reference(source, owner, known, lambda n: read(n).decode(),
                                             lambda n: (root / "Verify/current/vendor" / n).read_text())
                actual = read(standalone).decode()
                if remove_unprotected_blank_lines(actual) != remove_unprotected_blank_lines(reference):
                    issue("standalone_source_sync", standalone, dict(actual=digest(actual.encode()), reference=digest(reference.encode())))
                for directive in includes(actual):
                    if directive["name"] is None or directive["quoted"] or directive["name"].startswith("atcoder/"):
                        issue("standalone_unexpanded", standalone, directive["name"])
            except (ValueError, KeyError, UnicodeError, OSError) as error:
                issue("standalone_expansion", standalone, error)
        unresolved, links = 0, 0
        for name in files:
            if name.startswith("Examples/") and name.endswith(".cpp"):
                for directive in includes(read(name).decode()):
                    if directive["quoted"]:
                        try: target = relative_target(name, directive["name"])
                        except ValueError as error: issue("unsafe_example_include", name, error); continue
                        if target not in files: issue("example_dependency", name, target)
            if "/Verify/" in "/" + name and name.endswith(".cpp"):
                if files[name].file_size > VERIFY_LIMIT: issue("verify_size", name, files[name].file_size)
            if name.endswith(".md") and (name.startswith("Document/") or pathlib.PurePosixPath(name).name.lower() == "readme.md"):
                for destination in markdown_links(read(name).decode()):
                    parsed = urlsplit(destination)
                    if parsed.scheme or parsed.netloc or not parsed.path: continue
                    links += 1
                    try: target = relative_target(name, unquote(parsed.path).replace("\\(", "(").replace("\\)", ")"))
                    except ValueError as error: issue("unsafe_markdown_link", name, error); continue
                    if target not in files and not any(f.startswith(target.rstrip("/") + "/") for f in files):
                        unresolved += 1;issue("broken_markdown_link", name, dict(destination=destination, target=target))
        report["stats"].update(library_entries=len(selected), verify_entries=len(selected_verify) + (len(normal) if edition == "full" else 0), examples=len(examples), relative_markdown_links=links, broken_markdown_links=unresolved)
    report["ok"] = not report["errors"]
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=pathlib.Path, required=True)
    parser.add_argument("--archive", action="append", required=True, help="edition=ZIP; repeat for full+four editions")
    parser.add_argument("--output", type=pathlib.Path, required=True)
    args = parser.parse_args()
    reports = []
    for request in args.archive:
        edition, path = request.split("=", 1)
        reports.append(validate_archive(args.root, path, edition))
    args.output.write_text(json.dumps(dict(reports=reports, all_ok=all(r["ok"] for r in reports)), ensure_ascii=False, indent=2) + "\n")
    print(json.dumps([dict(edition=r["edition"], ok=r["ok"], errors=len(r["errors"]), warnings=len(r["warnings"])) for r in reports]))
    raise SystemExit(0 if all(r["ok"] for r in reports) else 1)


if __name__ == "__main__":
    main()
