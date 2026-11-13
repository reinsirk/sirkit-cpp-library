# SPDX-License-Identifier: CC0-1.0
"""同じPython/zlib版で、同じ名前と内容から同じZIPを作る。"""
from collections.abc import Mapping
from pathlib import Path
import platform
import stat
from tempfile import TemporaryDirectory
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo
import zlib


def archive_versions() -> dict[str, str]:
    """再現条件としてPythonとzlibの版を返す。版をまたぐ同一性は保証しない。"""
    return {"python": platform.python_version(), "zlib_compile": zlib.ZLIB_VERSION,
            "zlib_runtime": zlib.ZLIB_RUNTIME_VERSION}


def write_archive(output: Path, contents: Mapping[str, bytes]) -> None:
    """名前を検査してから固定日時・権限で作り、CRCと全内容を再読込して置き換える。"""
    entries = list(contents.items())
    seen: set[str] = set()
    for name, data in entries:
        if (not isinstance(name, str) or not name or "\\" in name or ":" in name or
                any(part in ("", ".", "..") for part in name.split("/")) or
                any(ord(c) < 32 or 127 <= ord(c) < 160 for c in name)):
            raise ValueError("ZIPの非正規・危険な名前: " + repr(name))
        if name in seen or len(name.encode("utf-8")) > 65535:
            raise ValueError("ZIPの重複または長すぎる名前: " + repr(name))
        if not isinstance(data, bytes):
            raise TypeError("ZIPの内容はbytesが必要: " + name)
        seen.add(name)
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix="sirkit-zip-", dir=output.parent) as directory:
        temporary = Path(directory) / "archive.zip"
        with ZipFile(temporary, "w", compression=ZIP_DEFLATED, compresslevel=6) as archive:
            for name, data in sorted(entries):
                info = ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
                info.create_system = 3
                info.external_attr = (stat.S_IFREG | 0o644) << 16
                archive.writestr(info, data, compress_type=ZIP_DEFLATED, compresslevel=6)
        with ZipFile(temporary) as archive:
            if archive.namelist() != sorted(seen):
                raise ValueError("ZIPのファイル一覧不一致")
            for name, data in entries:
                # readがCRCを検証する。同じ読込で元の内容とも比較する。
                if archive.read(name) != data:
                    raise ValueError("ZIP内容不一致: " + name)
        temporary.replace(output)
