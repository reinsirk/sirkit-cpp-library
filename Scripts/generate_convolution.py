#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""各種の畳み込みとワイルドカード照合の単独貼付版へ共通NTTを同期する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def render(root: Path) -> dict[Path,bytes]:
    ntt = unwrap_source((root / "Math/NTT.cpp").read_text())
    include_start = ntt.index("#include ")
    overview = ntt[:include_start].split("// API一覧\n", 1)[1].strip()
    ntt_code = ntt[include_start:].rstrip()
    output = {}
    for library, stem in (
        (root / "Math/Convolution.cpp", "convolution"),
        (root / "String/Wildcard_Pattern_Matching.cpp", "wildcard_pattern_matching"),
        (root / "Math/Multivariate_Convolution.cpp", "multivariate_convolution"),
        (root / "Math/Multiplicative_Convolution.cpp", "multiplicative_convolution"),
        (root / "Math/Consecutive_Polynomial_Evaluation.cpp", "consecutive_polynomial_evaluation"),
        (root / "Math/Polynomial_Sequence_Product.cpp", "polynomial_sequence_product"),
        (root / "Math/Geometric_Polynomial.cpp", "geometric_polynomial"),
        (root / "Math/Consecutive_Linear_Recurrence.cpp", "consecutive_linear_recurrence"),
    ):
        components = library.with_suffix("") / "components"
        header = (components / (stem + "_header.inc")).read_text().rstrip()
        body = (components / (stem + "_body.inc")).read_text().rstrip()
        data = f"{header}\n//\n// [同梱NTT]\n{overview}\n\n{ntt_code}\n\n{body}\n"
        output[library] = wrap_source(data).encode()
    return output

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    for path,data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.exists()or path.read_bytes()!=data:raise ValueError(str(path)+'が共通部品と同期していません')
        else:path.write_bytes(data)
    print('畳み込み: '+('同期確認済み'if args.check else'生成済み'))
if __name__=='__main__':main()
