# SPDX-License-Identifier: CC0-1.0
"""Verifyと性能比較で使うコンパイル引数。"""
from __future__ import annotations

from collections.abc import Sequence

COMPILE_PROFILES = ("standard", "atcoder")
ATCODER_CONSTEXPR_OPS_LIMIT = 2097152

# AtCoder C++23 (GCC 15.2.0) のソース・コード生成に関わる共通設定。
# https://img.atcoder.jp/file/language-update/2025-10/language-list.html
ATCODER_GCC_FLAGS = (
    "-std=gnu++23", "-O2", "-Wall", "-Wextra",
    "-DATCODER", "-DNOMINMAX", "-DONLINE_JUDGE",
    "-DOR_PROTO_DLL=", "-DPROTOBUF_USE_DLLS",
    "-DUSE_BOP", "-DUSE_CBC", "-DUSE_CLP", "-DUSE_GLOP", "-DUSE_LP_PARSER",
    "-DUSE_MATH_OPT", "-DUSE_PDLP", "-DUSE_SCIP",
    "-fconstexpr-depth=1024", "-fconstexpr-loop-limit=524288",
    f"-fconstexpr-ops-limit={ATCODER_CONSTEXPR_OPS_LIMIT}",
    "-fmodules", "-ftrivial-auto-var-init=zero", "-march=native", "-pthread",
    "-Wl,--as-needed", "-fopenmp", "-lstdc++exp",
)


def compile_flags(profile: str, standard_flags: Sequence[str]) -> list[str]:
    if profile == "standard":
        return list(standard_flags)
    if profile == "atcoder":
        extra = [flag for flag in standard_flags if flag != "-std=c++23" and flag not in ATCODER_GCC_FLAGS]
        return [*ATCODER_GCC_FLAGS, *extra]
    raise ValueError("unknown compile profile: " + profile)
