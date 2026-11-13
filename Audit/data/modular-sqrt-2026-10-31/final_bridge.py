from pathlib import Path
import subprocess,json,time,hashlib
p=Path(__file__).resolve().parent;source=p/'ready_Modular_Arithmetic.cpp';rows=[]
for r in json.loads((p/'gate_bench_build.json').read_text()):
 if r['candidate']!='sqrt_p8_narrow_ts':continue
 while(p/'pause').exists():(p/'pause_ack').touch();time.sleep(.1)
 (p/'pause_ack').unlink(missing_ok=True)
 exe=p/'validation'/f'finalbench-{r["compiler"]}-{r["native"]}';cmd=[str(exe)if a==r['binary']else('-DHEADER="'+str(source)+'"'if a.startswith('-DHEADER=')else a)for a in r['command']];run=subprocess.run(cmd,capture_output=True,text=True);assert not run.returncode,run.stderr
 sections={}
 for section in ['.text','.rodata','.eh_frame']:
  hashes=[]
  for file in [r['binary'],str(exe)]:
   out=p/'validation'/f'section-{r["compiler"]}-{r["native"]}-{Path(file).name}-{section[1:]}.bin';result=subprocess.run(['objcopy','--dump-section',section+'='+str(out),file],capture_output=True,text=True);assert not result.returncode,result.stderr;hashes.append(hashlib.sha256(out.read_bytes()).hexdigest())
  assert hashes[0]==hashes[1],(r['compiler'],r['native'],section,hashes);sections[section]=hashes
 rows.append({'compiler':r['compiler'],'native':r['native'],'source_measured_sha256':r['source_sha256'],'source_final_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'fixture_sha256':r['fixture_sha256'],'sections':sections,'all_sections_identical':True});(p/'final_sections.json').write_text(json.dumps(rows,indent=2));print(r['compiler'],r['native'],'ALL3_IDENTICAL',flush=True)
print('BRIDGE_PASS',len(rows),flush=True)
