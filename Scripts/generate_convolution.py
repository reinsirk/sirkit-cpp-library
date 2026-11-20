#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""任意法・整数畳み込みの単独貼付版へ共通NTTを同期する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def render(root: Path) -> dict[Path,bytes]:
    parts=root/'Math/Convolution/components'
    ntt=unwrap_source((root/'Math/NTT.cpp').read_text())
    overview=ntt[:ntt.index('#include ')].split('// API一覧\n',1)[1].strip()
    header=(parts/'convolution_header.inc').read_text().rstrip()+'\n//\n// [同梱NTT]\n'+overview
    data=header+'\n\n'+ntt[ntt.index('#include '):].rstrip()+'\n\n'+(parts/'convolution_body.inc').read_text().rstrip()+'\n'
    return {root/'Math/Convolution.cpp':wrap_source(data).encode()}

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    for path,data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.exists()or path.read_bytes()!=data:raise ValueError(str(path)+'が共通部品と同期していません')
        else:path.write_bytes(data)
    print('畳み込み: '+('同期確認済み'if args.check else'生成済み'))
if __name__=='__main__':main()
