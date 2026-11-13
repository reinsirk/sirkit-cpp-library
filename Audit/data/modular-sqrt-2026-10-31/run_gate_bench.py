from pathlib import Path
import subprocess,json,hashlib,resource,time,random
p=Path(__file__).resolve().parent;build=json.loads((p/'gate_bench_build.json').read_text());build +=[{**r,'candidate':'control_baseline'}for r in build if r['candidate']=='baseline_Mont'];build +=[{**r,'candidate':'control_gate'}for r in build if r['candidate']=='sqrt_p8_narrow_ts'];cases=json.loads((p/'benchmark_cases_extended.json').read_text())['cases'];rows=[];expected={};rng=random.Random(826134);start=time.time()
for case in cases:assert hashlib.sha256(Path(case['file']).read_bytes()).hexdigest()==case['sha256']
for repeat in range(3):
 tasks=[(r,case)for r in build for case in cases];rng.shuffle(tasks)
 for r,case in tasks:
  assert hashlib.sha256(Path(r['binary']).read_bytes()).hexdigest()==r['binary_sha256'];before=resource.getrusage(resource.RUSAGE_CHILDREN);started=time.perf_counter_ns();run=subprocess.run(['taskset','-c','12',r['binary'],case['file'],str(case['kind'])],capture_output=True,text=True,timeout=60);wall=time.perf_counter_ns()-started;after=resource.getrusage(resource.RUSAGE_CHILDREN);assert not run.returncode and not run.stderr,(r,case,run.stderr);values=run.stdout.strip().split(',');assert len(values)==5;expected.setdefault(case['id'],values[4]);assert expected[case['id']]==values[4],(r,case);rows.append({**r,'case':case,'repeat':repeat,'queries':int(values[1]),'wall_us':float(values[2]),'thread_us':float(values[3]),'checksum':values[4],'process_cpu_us':((after.ru_utime+after.ru_stime)-(before.ru_utime+before.ru_stime))*1e6,'process_wall_us':wall/1000})
 (p/'gate_bench_raw.json').write_text(json.dumps(rows,indent=2));print(repeat,len(rows),'PASS',flush=True)
(p/'gate_bench_manifest.json').write_text(json.dumps({'cpu':12,'started':start,'finished':time.time(),'rows':len(rows),'fixture_sha256':hashlib.sha256((p/'bench.cpp').read_bytes()).hexdigest()},indent=2));print('RAW_DONE',time.time()-start,flush=True)
