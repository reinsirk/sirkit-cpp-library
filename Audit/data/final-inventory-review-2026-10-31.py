# SPDX-License-Identifier: CC0-1.0
# Read-only final metadata correspondence; no builds or performance runs.
from pathlib import Path
from zipfile import ZipFile
import ast,collections,datetime,hashlib,json,re
ROOT=Path('/home/reinsirk/Atcoder_Env');OUT=Path('/tmp/final_metadata_review');WORK=Path('/tmp/sirkit_12h_20261031_095733')
sha=lambda data:hashlib.sha256(data).hexdigest()
load=lambda name:json.loads((ROOT/name).read_text())
inv=load('Audit/expansion-12hours-2026-10-31-inventory.json');normal=load('Verify/current/manifest.json')['entries'];profiles=load('Variants/verify_manifest.json');variants=load('Variants/manifest.json')['entries'];findings=[];checks={}
def check(name,okay,details=None):
 checks[name]={'pass':bool(okay),'details':details}
 if not okay:findings.append({'severity':'must_fix','check':name,'details':details})
def body(data):
 prefix=re.match(rb'(?:\s+|//[^\n]*(?:\n|$)|/\*[\s\S]*?\*/)*',data)
 return data[prefix.end():]
with ZipFile(WORK/'baseline.zip')as baseline:
 old_entries=load_old=json.loads(baseline.read('Verify/current/manifest.json'))['entries'];old={e['id']:e for e in old_entries};old_sources={n:baseline.read(n)for n in baseline.namelist()if n.endswith('.cpp')}
 old_drivers=[{'id':e['id'],'driver':e['driver'],'sha256':sha(baseline.read(e['driver'])),'current_sha256':sha((ROOT/e['driver']).read_bytes()),'metadata_old_sha256':e.get('driver_sha256')}for e in old_entries]
new_sources=sorted(({e['source']for e in variants}-old_sources.keys())|{'DataStructure/Fenwick_Tree.cpp','Graph/SCC.cpp'})
check('new_libraries_exact47',len(new_sources)==47 and new_sources==[e['source']for e in inv['new_libraries']],new_sources)
check('primary_sources105',len({e['source']for e in variants})==105,len({e['source']for e in variants}))
check('profile_sources420_and_unique',len(variants)==420 and len({(e['source'],e['compiler'],e['fast'])for e in variants})==420,len(variants))
check('profile_source_4profiles_each',all(len([e for e in variants if e['source']==s])==4 for s in {e['source']for e in variants}))
check('normal227',len(normal)==227,len(normal));check('profile_verify802',len(profiles['entries'])==802,len(profiles['entries']))
check('baseline_drivers148_byte_identical',len(old_drivers)==148 and all(e['sha256']==e['current_sha256']==e['metadata_old_sha256']for e in old_drivers),old_drivers)
new_online=[e for e in normal if e['id']not in old and e.get('problem_url')]
check('new_online79',len(new_online)==79 and {e['id']for e in new_online}=={e['id']for e in inv['normal_submission_priorities']['new_online_codes']},[e['id']for e in new_online])
changed={name:{'header_only':body(data)==body((ROOT/name).read_bytes()),'old_sha256':sha(data),'new_sha256':sha((ROOT/name).read_bytes())}for name,data in old_sources.items()if name in {e['source']for e in variants}and sha(data)!=sha((ROOT/name).read_bytes())}
classify={'body':[],'comments':[]}
for e in normal:
 if e['id']not in old or not e.get('problem_url'):continue
 altered=[s for s in e['libraries']if s in changed]
 if any(not changed[s]['header_only']for s in altered):classify['body'].append(e['id'])
 elif altered:classify['comments'].append(e['id'])
check('existing_body_change_HashMap_only',classify['body']==['hash_map'],classify['body'])
check('existing_comment_HashSet3_only',set(classify['comments'])=={'hash_set_membership','hash_set_toggle','hash_set_strings'},classify['comments'])
check('inventory_driver_changes_empty',inv['existing_main_driver_changes']==[],inv['existing_main_driver_changes'])
check('inventory_source_hashes_current',all(sha((ROOT/e['source']).read_bytes())==e['sha256']and(ROOT/e['source']).stat().st_size==e['bytes']for e in inv['new_libraries']))
check('inventory_manifest_hashes_current',all(sha((ROOT/n).read_bytes())==v for n,v in inv['current_manifest_sha256'].items()))
check('inventory_submission_hashes_current',all(sha((ROOT/e['submission']).read_bytes())==e['submission_sha256']for group in inv['normal_submission_priorities'].values()for e in group))
check('new_sources_CC0_SPDX',all((ROOT/s).read_text().startswith('// SPDX-License-Identifier: CC0-1.0')for s in new_sources))
check('new_source_docs_examples_exist',all((ROOT/e['document']).is_file()and e['examples']and all((ROOT/ex).is_file()for ex in e['examples'])for e in inv['new_libraries']))
fast=sorted({e['origin'] for e in variants if e['origin'].startswith('Fast/Source/')});check('dedicated_Fast_overrides11',len(fast)==11,{'public_source_overrides':fast,'additional_internal_helpers':[str(f.relative_to(ROOT))for f in (ROOT/'Fast/Source/Internal').glob('*.cpp')]})
for node in ast.parse((ROOT/'Scripts/package_revised_libraries.py').read_text()).body:
 if isinstance(node,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='LIBRARIES'for t in node.targets):example_map=ast.literal_eval(node.value)
check('examples103',len(example_map)==103,len(example_map))
profile_counts=[]
for compiler,fast in [('GCC',False),('Clang',False),('GCC',True),('Clang',True)]:
 es=[e for e in profiles['entries']if e['compiler']==compiler and e['fast']==fast];profile_counts.append({'compiler':compiler,'fast':fast,'online':sum(bool(e.get('problem_url'))for e in es),'local':sum(not bool(e.get('problem_url'))for e in es)})
check('profile_counts_173online_54or1local',all(e['online']==173 and e['local']==(1 if e['fast'] else 54)for e in profile_counts),profile_counts)
check('Fast_local_omission106',len(profiles['omitted_local'])==106,len(profiles['omitted_local']))
links=[]
for name in ['Audit/expansion-12hours-2026-10-31-summary.md','Audit/submission-checklist-12hours-2026-10-31.md']:
 text=(ROOT/name).read_text()
 for target in re.findall(r'\]\(([^)]+)\)',text):
  if '://'in target or target.startswith('#'):continue
  path=(ROOT/name).parent/target.split('#')[0];links.append({'owner':name,'target':target,'exists':path.exists()})
check('summary_checklist_local_links',all(x['exists']for x in links),links)
checklist=(ROOT/'Audit/submission-checklist-12hours-2026-10-31.md').read_text();newsection=checklist.split('## 新規に用意したコード\n')[1].split('\n## ')[0]
listed=re.findall(r'\| \[([^\]]+)\]\(../Verify/current/submissions/',newsection)
check('checklist_exact79_unique',len(listed)==len(set(listed))==79 and set(listed)=={e['id']for e in new_online},listed)
newnormal=load('Audit/normal-verify-final-release-sqrt-2026-10-31.json');programs=load('Audit/variant-programs-local-validation-2026-10-31.json');cpp=load('Audit/verify-compiler-variants-current-2026-10-31.json');combinations=load('Audit/verify-library-combinations-current-2026-10-31.json');runtime=load('Audit/runtime-coexistence-2026-10-31.json');simd=load('Audit/runtime-simd-dispatch-2026-10-31.json');convolution=load('Audit/convolution-maximum-capacity-2026-10-31.json')
check('normal_gate1816_3184',newnormal['conditions']==1816 and newnormal['sample_runs']==3184 and newnormal['normal_entries']==227 and newnormal['status']=='LOCAL_SAMPLE_PASS_FOR_FROZEN_NORMAL_SNAPSHOT')
check('variant_programs2840allPASS',len(programs['results'])==2840 and all(e['status']=='LOCAL_SAMPLE_PASS'for e in programs['results']))
check('compiler1470',len(cpp['results'])==1470 and cpp['status']=='passed')
check('combination72',len(combinations['results'])==72 and combinations['status']=='passed')
check('runtime330_distinct304',runtime['validation']['configurations']==330 and runtime['validation']['distinct_code_flags_compiler_keys']==304)
check('runtime16_origins',len({e['origin']for e in runtime['snapshot']['sources']})==16)
check('simd5sources20conditions',len(simd['scope'])==5 and len(simd['results'])==20 and simd['summary']['passed']==20 and simd['not_hardware_emulation'])
check('convolution50331648_2pow23',convolution['total_exact_coefficient_checks']==50331648 and convolution['maximum_transform_length']==2**23 and convolution['status']=='passed')
check('current_cpp_manifest_evidence',cpp['manifest_sha256']==sha((ROOT/'Variants/manifest.json').read_bytes()))
# Frozen policy review predates final Sqrt; do not reinterpret it as a newer audit.
policy=load('Audit/final-policy-provenance-47-libraries-2026-10-31.json')
policy_drift=[e['path']for e in policy['source_inventory']if (ROOT/e['path']).is_file()and sha((ROOT/e['path']).read_bytes())!=e['sha256']]
check('policy47normal11fast_scope',policy['scope']['normal_count']==47 and policy['scope']['fast_count']==11 and not policy['unresolved_findings'])
findings +=[
 {'severity':'finalize_before_release','path':'Audit/expansion-12hours-2026-10-31-inventory.json','detail':'status is still in_progress_until_2026-10-31T21:57:33Z. It is accurate while work remains active, but set final status/completion time at actual completion.'},
 {'severity':'finalize_before_release','path':'Audit/expansion-12hours-2026-10-31-summary.md','detail':'ZIP paragraph still says final ZIP evidence will be added after generation. Replace with final ZIP/CRC/independent-check receipt links and actual final status before final archive freeze.'},
 {'severity':'scope_note','path':'Audit/final-policy-provenance-47-libraries-2026-10-31.json','detail':'Policy review is a frozen earlier review, not exact final-source review. Source drift: '+', '.join(policy_drift)+'. Summary already limits runtime-coexistence to frozen hashes; keep policy scope/date exact, append a supplemental review receipt if claiming all final bytes.'}
]
tracked=['Audit/expansion-12hours-2026-10-31-summary.md','Audit/expansion-12hours-2026-10-31-inventory.json','Audit/submission-checklist-12hours-2026-10-31.md','Variants/manifest.json','Variants/verify_manifest.json','Verify/current/manifest.json']
report={'at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'read_only':True,'source_changes':False,'builds_or_performance_runs':False,'checks':checks,'must_fix_count':sum(e['severity']=='must_fix'for e in findings),'findings':findings,'source_changes_since_backup':changed,'frozen_input_sha256':{n:sha((ROOT/n).read_bytes())for n in tracked}}
(OUT/'review.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print('CHECKS',len(checks),'PASS',sum(x['pass']for x in checks.values()),'MUST_FIX',report['must_fix_count']);print('POLICY_DRIFT',policy_drift);print('FINDINGS',json.dumps(findings,ensure_ascii=False))
