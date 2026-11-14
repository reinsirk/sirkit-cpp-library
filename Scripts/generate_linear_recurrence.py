#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""線形漸化式の単独貼付版へ共通NTTとBMを同期する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def render(root: Path) -> dict[Path, bytes]:
    parts=root/'Math/Recurrence/components'
    coefficient=(parts/'recurrence_coefficient.inc').read_text()
    bm=(parts/'berlekamp_massey_body.inc').read_text()
    ntt=unwrap_source((root/'Math/NTT.cpp').read_text())
    ntt_overview=ntt[:ntt.index('#include ')].split('// API一覧\n',1)[1].strip()
    ntt=ntt[ntt.index('#include '):]
    bm_overview=(parts/'berlekamp_massey_header.inc').read_text().split('// API一覧\n',1)[1].strip()
    linear_header=(parts/'linear_recurrence_header.inc').read_text().rstrip()+'\n//\n// [同梱BerlekampMassey]\n'+bm_overview+'\n//\n// [同梱NTT]\n'+ntt_overview
    return {
        root/'Math/Berlekamp_Massey.cpp': wrap_source((parts/'berlekamp_massey_header.inc').read_text().rstrip()+'\n\n'+coefficient.rstrip()+'\n\n'+bm.rstrip()+'\n').encode(),
        root/'Math/Linear_Recurrence.cpp': wrap_source(linear_header+'\n\n'+(parts/'recurrence_montgomery.inc').read_text().rstrip()+'\n\n'+ntt.rstrip()+'\n\n'+coefficient.rstrip()+'\n\n'+bm.rstrip()+'\n\n'+(parts/'linear_recurrence_body.inc').read_text().rstrip()+'\n').encode(),
    }

def main() -> None:
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    for path,data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.exists()or path.read_bytes()!=data:raise ValueError(str(path)+'が共通部品と同期していません')
        else:path.write_bytes(data)
    print('線形漸化式: '+('同期確認済み'if args.check else'生成済み'))
if __name__=='__main__':main()
