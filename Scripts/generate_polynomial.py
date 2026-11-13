#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""多点評価・補間の単独貼付版へ共通FPS/NTTを同期する。"""
from pathlib import Path
import argparse

def render(root: Path) -> dict[Path,bytes]:
    parts=root/'Math/Polynomial/components';fps=(root/'Math/Formal_Power_Series.cpp').read_text()
    overview=fps[:fps.index('#include ')].split('// API一覧\n',1)[1].strip()
    header=(parts/'polynomial_header.inc').read_text().rstrip()+'\n//\n// [同梱FPS/NTT]\n'+overview
    data=header+'\n\n'+fps[fps.index('#include '):].rstrip()+'\n\n'+(parts/'polynomial_body.inc').read_text().rstrip()+'\n'
    return {root/'Math/Polynomial.cpp':data.encode()}

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    for path,data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.exists()or path.read_bytes()!=data:raise ValueError(str(path)+'が共通部品と同期していません')
        else:path.write_bytes(data)
    print('多項式: '+('同期確認済み'if args.check else'生成済み'))
if __name__=='__main__':main()
