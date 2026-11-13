from pathlib import Path
import subprocess,json,time,hashlib
p=Path(__file__).resolve().parent;out=p/'validation';out.mkdir(exist_ok=True);rows=[]
for compiler,command in [('gcc',['/usr/bin/g++']),('clang',['/usr/bin/clang++','--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15'])]:
 for native in [False,True]:
  for candidate in ['baseline_Mont','sqrt_p8_encoded','sqrt_p8_typed','sqrt_p8_narrow_ts']:
   while(p/'pause').exists():(p/'pause_ack').touch();time.sleep(.1)
   (p/'pause_ack').unlink(missing_ok=True)
   exe=out/f'gatebench-{compiler}-{native}-{candidate}';cmd=['taskset','-c','4']+command+['-std=c++23','-O2','-DSKIP_RUNTIME_CHECKS','-DHEADER="'+str(p/(candidate+'.hpp'))+'"']+(['-march=native']if native else[])+[str(p/'bench.cpp'),'-o',str(exe)];build=subprocess.run(cmd,capture_output=True,text=True);row={'compiler':compiler,'native':native,'candidate':candidate,'command':cmd,'source_sha256':hashlib.sha256((p/(candidate+'.hpp')).read_bytes()).hexdigest(),'fixture_sha256':hashlib.sha256((p/'bench.cpp').read_bytes()).hexdigest(),'build_exit':build.returncode,'diagnostics':build.stderr,'binary':str(exe)}
   if not build.returncode:row['binary_sha256']=hashlib.sha256(exe.read_bytes()).hexdigest()
   rows.append(row);(p/'gate_bench_build.json').write_text(json.dumps(rows,indent=2));print(compiler,native,candidate,build.returncode,flush=True)
   if build.returncode:raise SystemExit(1)
