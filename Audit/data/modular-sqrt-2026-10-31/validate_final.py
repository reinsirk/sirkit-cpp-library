from pathlib import Path
import subprocess,json,time,hashlib
p=Path(__file__).resolve().parent;source=p/'ready_Modular_Arithmetic.cpp';testdir=p/'ready/Verify/current/tests';rows=[]
for compiler,command in [('gcc',['/usr/bin/g++']),('clang',['/usr/bin/clang++','--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15'])]:
 for fixture in ['reference','allocation','constexpr','coexist']:
  file=testdir/f'modular_sqrt_{fixture}_test.cpp' if fixture!='coexist' else Path('/home/reinsirk/Atcoder_Env/Verify/current/tests/modular_montgomery_coexist_test.cpp')
  modes=[(standard,skip,False,last)for standard in [False,True]for skip in [False,True]for last in ([False,True]if fixture=='coexist'else[False])]
  if fixture=='reference':modes +=[(standard,False,True,False)for standard in [False,True]]
  for standard,skip,sanitize,last in modes:
   while(p/'pause').exists():(p/'pause_ack').touch();time.sleep(.1)
   (p/'pause_ack').unlink(missing_ok=True)
   flags=['-std=c++23','-O2','-DMODULAR_SOURCE="'+str(source)+'"']+(['-DSKIP_RUNTIME_CHECKS']if skip else[])+(['-DSIRKIT_FORCE_STANDARD_ARITHMETIC']if standard else[])+(['-DSIRKIT_TEST_MODULAR_ARITHMETIC_LAST']if last else[])+(['-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']if sanitize else[])
   exe=p/'validation'/f'final-{compiler}-{fixture}-{standard}-{skip}-{sanitize}-{last}';run=subprocess.run(['taskset','-c','4']+command+flags+[str(file),'-o',str(exe)],capture_output=True,text=True);row={'compiler':compiler,'fixture':fixture,'standard':standard,'skip':skip,'sanitize':sanitize,'last':last,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'fixture_sha256':hashlib.sha256(file.read_bytes()).hexdigest(),'build_exit':run.returncode,'diagnostics':run.stderr}
   if not run.returncode:
    run=subprocess.run(['taskset','-c','4',str(exe)],capture_output=True,text=True,env={**__import__('os').environ,'ASAN_OPTIONS':'detect_leaks=0'});row.update(run_exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
   rows.append(row);(p/'final_results.json').write_text(json.dumps(rows,indent=2));print(compiler,fixture,standard,skip,sanitize,last,row.get('run_exit',1),flush=True)
   if row['build_exit']or row.get('run_exit',1):raise SystemExit(1)
print('FINAL_PASS',len(rows),flush=True)
