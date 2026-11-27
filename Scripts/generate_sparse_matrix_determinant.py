#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""疎行列式の単独貼付版へ既存のBerlekamp–Masseyを同梱する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


def render(root: Path) -> dict[Path, bytes]:
    part = root / "Math/SparseMatrixDeterminant/components/sparse_matrix_determinant.inc"
    source = part.read_text()
    split = source.index("#include ")
    bm = unwrap_source((root / "Math/Berlekamp_Massey.cpp").read_text())
    bm_split = bm.index("#include ")
    overview = bm[:bm_split].split("// API一覧\n", 1)[1].strip()
    content = f"{source[:split].rstrip()}\n//\n// [同梱Berlekamp–Massey]\n{overview}\n\n{bm[bm_split:].rstrip()}\n\n{source[split:].rstrip()}\n"
    return {root / "Math/Sparse_Matrix_Determinant.cpp": wrap_source(content).encode()}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for path, data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.is_file() or path.read_bytes() != data:
                raise ValueError(str(path) + "が共通部品と同期していません")
        else:
            path.write_bytes(data)


if __name__ == "__main__":
    main()
