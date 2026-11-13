# SPDX-License-Identifier: CC0-1.0
# Read-only immutable Audit/data artifact reference checker. No source-version comparisons.
from pathlib import Path
from zipfile import ZipFile
import collections,datetime,hashlib,json,os,re
ROOT=Path('/home/reinsirk/Atcoder_Env');DATA=ROOT/'Audit/data';OUT=Path('/tmp/final_artifact_reference_check');BASE=Path('/tmp/sirkit_12h_20261031_095733/baseline.zip')
os.sched_setaffinity(0,{4})
def hash_file(path):
 h=hashlib.sha256()
 with Path(path).open('rb')as file:
  while chunk:=file.read(1024*1024):h.update(chunk)
 return h.hexdigest()
HEX=re.compile(r'^[0-9a-fA-F]{64}$');refs=[];unknown=[];excluded=collections.Counter();parse_errors=[];schemas=collections.Counter()
def digest(value):return isinstance(value,str)and bool(HEX.fullmatch(value))
def size(record):
 for name in ['bytes','size_bytes','size']:
  v=record.get(name)
  if isinstance(v,int)and not isinstance(v,bool)and v>=0:return v
 return None
def target(value,owner,relative_manifest=False):
 if not isinstance(value,str)or not value or '\x00'in value:return None
 if value.startswith('Audit/data/'):p=ROOT/value
 elif value.startswith(str(DATA)+'/'):p=Path(value)
 elif value.startswith('data/')and owner.parent==ROOT/'Audit':p=owner.parent/value
 elif relative_manifest and not Path(value).is_absolute():p=owner.parent/value
 else:return None
 if '\\'in value or any(x in ('.','..')for x in Path(value).parts):return None
 try:p.resolve().relative_to(DATA.resolve())
 except ValueError:return None
 return p

def emit(owner,pointer,path,sha,expected_size,schema):
 schemas[schema]+=1;refs.append({'owner':owner.relative_to(ROOT).as_posix(),'json_pointer':pointer,'target':path.relative_to(ROOT).as_posix(),'expected_sha256':sha.lower(),'expected_bytes':expected_size,'schema':schema})
def walk(value,owner,pointer='',parent_key=None):
 if isinstance(value,dict):
  matched=set()
  # Direct record: {path/file/source/...:Audit/data/...,sha256:...,bytes?:...}
  for field,v in value.items():
   p=target(v,owner)
   if p is None:continue
   if digest(value.get('sha256')):
    emit(owner,pointer+'/'+str(field),p,value['sha256'],size(value),'record_path_sha256');matched.add(field)
   else:
    stem=field[:-5]if field.endswith('_path')else field
    hashkey=stem+'_sha256'
    if digest(value.get(hashkey)):
     expected_size=next((value[k]for k in [stem+'_bytes',stem+'_size_bytes',stem+'_size']if isinstance(value.get(k),int)and not isinstance(value.get(k),bool)),None)
     emit(owner,pointer+'/'+str(field),p,value[hashkey],expected_size,'sibling_named_path_sha256');matched.add(field)
  # Path-keyed mapping: {Audit/data/file:SHA} or {path:{sha256,bytes}}.
  # Known directory manifest schema: Audit/data/**/manifest.json files:{relativeName:{sha256,bytes}}.
  manifest_relative=owner.is_relative_to(DATA)and owner.name=='manifest.json'and parent_key=='files'
  for field,v in value.items():
   p=target(field,owner,relative_manifest=manifest_relative)
   if p is not None:
    h=v if digest(v)else v.get('sha256')if isinstance(v,dict)else None
    if digest(h):emit(owner,pointer+'/'+str(field),p,h,size(v)if isinstance(v,dict)else None,'relative_files_manifest'if manifest_relative else'path_keyed_sha256');matched.add(field)
    else:unknown.append({'owner':owner.relative_to(ROOT).as_posix(),'json_pointer':pointer+'/'+str(field),'reference':field,'reason':'data path key without recognized SHA record'})
   if field not in matched and target(v,owner)is not None:
    unknown.append({'owner':owner.relative_to(ROOT).as_posix(),'json_pointer':pointer+'/'+str(field),'reference':v,'reason':'data path value withoutsame-record SHA256 or matching named sibling'})
   walk(v,owner,pointer+'/'+str(field),field)
 elif isinstance(value,list):
  for i,v in enumerate(value):walk(v,owner,pointer+'/'+str(i),None)
# Probe recognition only; synthetic paths never become artifact-file checks.
probe_owner=DATA/'_schema_probe/manifest.json';probe_hash='a'*64
probes=[({'files':{'x.hpp':{'sha256':probe_hash,'bytes':7}}},probe_owner,1),({'files':[{'path':'Audit/data/x.zip','sha256':probe_hash,'bytes':7}]},probe_owner,1),({'artifacts':{'Audit/data/x.zip':probe_hash}},ROOT/'Audit/probe.json',1),({'path':'Audit/data/x.zip','sha256':probe_hash,'bytes':7},ROOT/'Audit/probe.json',1),({'raw':'Audit/data/x.zip','raw_sha256':probe_hash},ROOT/'Audit/probe.json',1),({'path':'/tmp/oldsource.cpp','sha256':probe_hash},ROOT/'Audit/probe.json',0),({'source':'Math/NTT.cpp','sha256':probe_hash},ROOT/'Audit/probe.json',0)]
for value,owner,want in probes:
 refs.clear();unknown.clear();schemas.clear();walk(value,owner);assert len(refs)==want,(value,refs)
refs.clear();unknown.clear();schemas.clear()
with ZipFile(BASE)as archive:old=set(archive.namelist())
all_json=sorted((ROOT/'Audit').rglob('*.json'));new=[p for p in all_json if p.relative_to(ROOT).as_posix()not in old]
# The baseline intentionally omitted many older Audit files. Date scope prevents treating
# old Oct17–30 histories as newly created during this12hour goal.
selected=[p for p in new if'2026-10-31'in p.relative_to(ROOT).as_posix()]
inputs=[];input_drift=[]
for owner in selected:
 try:
  before_stat=owner.stat()
  with owner.open()as file:value=json.load(file)
  input_hash=hash_file(owner);after_stat=owner.stat()
  if(before_stat.st_size,before_stat.st_mtime_ns)!=(after_stat.st_size,after_stat.st_mtime_ns):input_drift.append(owner.relative_to(ROOT).as_posix())
  inputs.append({'path':owner.relative_to(ROOT).as_posix(),'bytes':after_stat.st_size,'sha256':input_hash})
  walk(value,owner)
 except(json.JSONDecodeError,UnicodeDecodeError)as error:parse_errors.append({'path':str(owner.relative_to(ROOT)),'error':str(error)})
cache={}
for ref in refs:
 p=ROOT/ref['target']
 if ref['target']not in cache:
  if not p.is_file():cache[ref['target']]={'exists':False,'regular_file':False}
  else:
   h=hashlib.sha256();count=0
   with p.open('rb')as file:
    while chunk:=file.read(1024*1024):h.update(chunk);count+=len(chunk)
   cache[ref['target']]={'exists':True,'regular_file':True,'actual_bytes':count,'actual_sha256':h.hexdigest()}
 actual=cache[ref['target']];ref['actual']=actual;ref['status']='PASS'if actual['exists']and actual['actual_sha256']==ref['expected_sha256']and(ref['expected_bytes']is None or ref['expected_bytes']==actual['actual_bytes'])else'FAIL'
fail=[r for r in refs if r['status']!='PASS'];report={'at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'read_only':True,'cpu':4,'chunk_bytes':1024*1024,'source_version_comparisons':False,'schema_probes_passed':7,'input_stat_drift':input_drift,'no_new_measurements':True,'scope':{'all_Audit_JSON':len(all_json),'absent_from_baseline':len(new),'selected_goal_dated_JSON':len(selected),'selected_input_bytes':sum(e['bytes']for e in inputs),'selection_rule':'Absent from baseline.zip AND repositoryrelativepathcontains2026-10-31; old Audit dates deliberately excluded because baseline omitted historicalAudit. Nested artifacts with arbitrarybasenames in dateddata directories are included.','baseline_sha256':hash_file(BASE)},'recognized_schemas':dict(schemas),'recognized_references':len(refs),'unique_artifact_files':len(cache),'unique_artifact_bytes':sum(e.get('actual_bytes',0)for e in cache.values()),'pass_references':len(refs)-len(fail),'failed_references':len(fail),'failures':fail,'unsupported_or_out_of_scope':{'recognized_data_paths_without_associated_SHA':unknown,'not_compared':['/tmp historical measurement paths, currentlibrary code sourceSHA and baselinesourceSHA','nonAudit/data files includingAudit/Measurements andScripts/Variants','arbitrary relativepaths outsideknown data-directory manifest.files schema','ZIP/gzip internalcontent semantics, declaredcoefficient correctness, binary buildIDs','opaqueembeddedJSON/source strings; no interpretation or code execution','older datedAudits not belonging to2026-10-31goal'],'parse_errors':parse_errors},'inputs':inputs,'references':refs}
report['size_metadata_present_references']=sum(r['expected_bytes']is not None for r in refs)
report['sha_only_references']=len(refs)-report['size_metadata_present_references']
report['recognized_schemas']={name:report['recognized_schemas'].get(name,0)for name in ['path_keyed_sha256','record_path_sha256','sibling_named_path_sha256','relative_files_manifest']}
OUT.mkdir(exist_ok=True);(OUT/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print('SCANNED',len(selected),'JSON','REFS',len(refs),'UNIQUE',len(cache),'PASS',len(refs)-len(fail),'FAIL',len(fail),'UNKNOWN',len(unknown),'PARSE_ERRORS',len(parse_errors),flush=True)
for e in fail:print('FAIL',e['owner'],e['json_pointer'],e['target'],e['expected_sha256'],e['actual'],flush=True)
