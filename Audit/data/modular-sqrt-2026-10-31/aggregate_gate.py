from pathlib import Path
import json,statistics,collections,math
p=Path(__file__).resolve().parent;raw=json.loads((p/'gate_bench_raw.json').read_text());groups=collections.defaultdict(lambda:collections.defaultdict(list));case_by={}
for row in raw:groups[(row['compiler'],row['native'],row['case']['id'])][row['candidate']].append(row);case_by[row['case']['id']]=row['case']
summary=[]
for key,g in sorted(groups.items()):
 medians={name:{metric:statistics.median(x[metric]for x in rows)for metric in ['thread_us','wall_us','process_cpu_us','process_wall_us']}for name,rows in g.items()};ratios={name:{metric:v[metric]/medians['baseline_Mont'][metric]for metric in v}for name,v in medians.items()};summary.append({'condition':key,'case':case_by[key[2]],'medians':medians,'ratios_to_baseline':ratios})
(p/'gate_bench_summary.json').write_text(json.dumps(summary,indent=2))
for compiler in ['gcc','clang']:
 for native in [False,True]:
  print(compiler,'native'if native else'default')
  for row in summary:
   if row['condition'][:2]!=[compiler,native]and tuple(row['condition'][:2])!=(compiler,native):continue
   case=row['case'];r=row['ratios_to_baseline'];print('pow'if case['kind']else'sqrt',case['shape'],case['pattern'],' '.join(name+':%.3f'%r[name]['thread_us']for name in ['sqrt_p8_encoded','sqrt_p8_typed','sqrt_p8_narrow_ts']),'ctrl %.3f/%.3f'%(r['control_baseline']['thread_us'],r['control_gate']['thread_us']))
