from pathlib import Path
from zipfile import ZipFile
import datetime,hashlib,json,time
ROOT=Path('/home/reinsirk/Atcoder_Env')
OUT=Path('/tmp/sirkit_12h_20261031_095733/final_freeze_monitor');OUT.mkdir(exist_ok=True)
END=datetime.datetime.fromisoformat('2026-10-31T21:57:33+00:00').timestamp()
def sha(data):return hashlib.sha256(data).hexdigest()
def now():return datetime.datetime.now(datetime.timezone.utc).isoformat()
def save(path,obj):
 tmp=path.with_suffix('.tmp');tmp.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+'\n');tmp.replace(path)
archive=ROOT/'Distribution/sirkit-cpp-library_full.zip'
with ZipFile(archive)as z:
 expected={name:dict(sha256=sha(z.read(name)),bytes=z.getinfo(name).file_size)for name in z.namelist()if not name.startswith(('Audit/','StandaloneExamples/'))}
save(OUT/'expected.json',expected)
report=dict(started_utc=now(),deadline_utc='2026-10-31T21:57:33Z',scope='Final full ZIPのnon-Audit/non-Standalone entriesに対応するworkspace bytesを監視する。新しいC++テストや性能測定ではない。',files=len(expected),total_bytes=sum(e['bytes']for e in expected.values()),archive_sha256=sha(archive.read_bytes()),checks=[],status='running')
print('Watching frozen source/tool/docs:',len(expected),'files',flush=True)
while True:
 changed=[]
 for name,e in expected.items():
  p=ROOT/name
  if not p.is_file():changed.append(dict(path=name,reason='missing'));continue
  actual=sha(p.read_bytes())
  if actual!=e['sha256']:changed.append(dict(path=name,expected=e['sha256'],actual=actual))
 report['checks'].append(dict(at_utc=now(),changed=changed));report['updated_utc']=now()
 if changed:
  report['status']='drift_detected';save(OUT/'report.json',report);print(json.dumps(changed),flush=True);raise SystemExit(2)
 if time.time()>=END:
  report['status']='stable_until_deadline';report['completed_utc']=now();save(OUT/'report.json',report);print('No source/tool/doc drift:',len(report['checks']),'checks',flush=True);break
 save(OUT/'report.json',report)
 time.sleep(min(45,max(0,END-time.time())))
