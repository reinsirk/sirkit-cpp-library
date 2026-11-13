# SPDX-License-Identifier: CC0-1.0
# Frozen Audit folder is read-only evidence. Copy inputs/scripts to a fresh destination.
from pathlib import Path
import argparse,json,shutil
parser=argparse.ArgumentParser();parser.add_argument('destination');args=parser.parse_args();source=Path(__file__).resolve().parent;target=Path(args.destination).resolve();target.mkdir(parents=True,exist_ok=False)
for name in ['baseline_Mont.hpp','sqrt_p8_encoded.hpp','sqrt_p8_typed.hpp','sqrt_p8_narrow_ts.hpp','bench.cpp','build_gate_bench.py','run_gate_bench.py','aggregate_gate.py']:
 shutil.copyfile(source/name,target/name)
(target/'validation').mkdir();shutil.copytree(source/'data',target/'data');cases=json.loads((source/'benchmark_cases_extended.json').read_text())
for case in cases['cases']:case['file']=str(target/'data'/Path(case['file']).name)
(target/'benchmark_cases_extended.json').write_text(json.dumps(cases,indent=2))
print('Prepared',target)
print('Build: python3',target/'build_gate_bench.py')
print('Run (quiet CPU12 window): python3',target/'run_gate_bench.py')
print('Aggregate after raw completion: python3',target/'aggregate_gate.py')
print('Build uses CPU4; benchmark uses CPU12. Adjust only affinity if reproducing on another host; record it.')
