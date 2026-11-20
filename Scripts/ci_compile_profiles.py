# SPDX-License-Identifier: CC0-1.0
"""Verifyと性能比較で使うコンパイル引数。"""
from __future__ import annotations

from collections.abc import Sequence

COMPILE_PROFILES = ("standard", "atcoder")
VERIFY_COMPILE_PROFILES = (*COMPILE_PROFILES, "atcoder-clang")
ATCODER_CONSTEXPR_OPS_LIMIT = 2097152

# AtCoder C++23 のソース・コード生成に関わる設定。
# https://img.atcoder.jp/file/language-update/2025-10/language-list.html
ATCODER_COMMON_FLAGS = (
    "-std=gnu++23", "-O2", "-Wall", "-Wextra",
    "-DATCODER", "-DNOMINMAX", "-DONLINE_JUDGE",
    "-DOR_PROTO_DLL=", "-DPROTOBUF_USE_DLLS",
    "-DUSE_BOP", "-DUSE_CBC", "-DUSE_CLP", "-DUSE_GLOP", "-DUSE_LP_PARSER",
    "-DUSE_MATH_OPT", "-DUSE_PDLP", "-DUSE_SCIP",
    "-fconstexpr-depth=1024",
)
ATCODER_GCC_FLAGS = (
    *ATCODER_COMMON_FLAGS, "-fconstexpr-loop-limit=524288",
    f"-fconstexpr-ops-limit={ATCODER_CONSTEXPR_OPS_LIMIT}",
    "-fmodules", "-ftrivial-auto-var-init=zero", "-march=native", "-pthread",
    "-Wl,--as-needed", "-fopenmp", "-lstdc++exp",
)
ATCODER_CLANG_FLAGS = (
    *ATCODER_COMMON_FLAGS,
    "-Wno-float-conversion", "-Wno-implicit-float-conversion",
    "-Wno-implicit-int-float-conversion", "-Wno-unknown-warning-option",
    "-fconstexpr-steps=524288", "-fexperimental-library", "-flto=auto",
    "-fprebuilt-module-path=.", "-ftrivial-auto-var-init=zero", "-fuse-ld=lld",
    "-march=native", "-pthread", "-rtlib=compiler-rt", "-stdlib=libc++",
    "-unwindlib=libunwind", "-Wl,--as-needed", "-fopenmp", "-fopenmp-extensions",
)


def compile_flags(profile: str, standard_flags: Sequence[str]) -> list[str]:
    if profile == "standard":
        return list(standard_flags)
    if profile == "atcoder":
        atcoder = ATCODER_GCC_FLAGS
    elif profile == "atcoder-clang":
        atcoder = ATCODER_CLANG_FLAGS
    else:
        raise ValueError("unknown compile profile: " + profile)
    extra = [flag for flag in standard_flags if flag != "-std=c++23" and flag not in atcoder]
    return [*atcoder, *extra]
