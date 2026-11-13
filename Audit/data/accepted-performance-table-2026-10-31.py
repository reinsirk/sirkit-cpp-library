# SPDX-License-Identifier: CC0-1.0
# Existing measurements only; no measurements or formal-source edits.
from pathlib import Path
import datetime,hashlib,json
R=Path('/home/reinsirk/Atcoder_Env');O=Path('/tmp/final_performance_table');settings=[('gcc',False),('gcc',True),('clang',False),('clang',True)];rows=[]
load=lambda p:json.loads((R/p).read_text())
def rec(compiler,native,ratio,before=None,after=None,selector=None):return {'compiler':compiler,'native':native,'after_over_before':ratio,'before':before,'after':after,'audit_selector':selector}
def add(name,workload,unit,before,records,gcc,clang,caveat,audits,scope):rows.append({'name':name,'representative_workload':workload,'metric_and_unit':unit,'comparison_baseline':before,'records':records,'table_gcc_default_native':gcc,'table_clang_default_native':clang,'major_regression_or_limit':caveat,'audits':audits,'scope':scope})
# 1: One fixed large convolution, not best of all operations.
p='Audit/performance-ntt-simd-2026-10-31.json';a=load(p)['summary']['conditions'];es=[e for e in a if e['operation']=='normalized_move'and e['n']==e['m']==524288];rs=[rec('gcc'if e['mode'].startswith('gcc')else'clang',e['mode'].endswith('native'),e['thread_ratio'],e['thread_baseline_us'],e['thread_median_us'],{'operation':'normalized_move','n':524288,'m':524288,'mode':e['mode']})for e in es]
add('NTT / FPS','法998244353、cache準備済み524288×524288のnormalized_move畳み込み','body threadCPU、1畳み込みのμs','NTT baseline26d3…→採用候補6fa9…（最終注記bridge）',rs,'0.401 / 0.381','0.376 / 0.389','長さ16逆変換でClang+17.4ns。FPS単体の数値は未測定。',[p,'Audit/ntt-simd-adoption-2026-10-31.json'],'FPSは同じNTT本文を継承しFPS部品は不変。4設定それぞれ同じ代表条件、5標本中央値。')
# 2: ordinary and Fast against same old ordinary baseline.
p='Audit/performance-persistent-segment-tree-iterative-2026-10-31.json';a=load(p)['measurements']['summary'];rs=[]
for e in a:
 if e['n']==65536 and e['q']==120000 and e['operation']==1 and e['label']in ['normal_iterative','vector_raw_iterative']:rs.append({**rec(e['compiler'],e['native'],e['ratios']['operation'],selector={'n':65536,'q':120000,'operation':1,'label':e['label']}),'variant':'Fast'if e['label']=='vector_raw_iterative'else'normal'})
add('PST（通常 / Fast）','n65536、prod120000回、operation1（更新なし）','prod phase threadCPU比','同じ旧通常版PST→反復prodの通常/速度優先版',rs,'通常0.499/0.478、Fast0.474/0.488','通常0.523/0.531、Fast0.544/0.528','構築・破棄の小区間は後退もある。Fastが常に通常版より速いわけではない。',[p],'Fastは状態を解放しないがノードvectorは通常成長。比は両方とも同じ旧通常版に対する値。')
# 3: final default candidate build measurement and separately scoped native query long-run range.
p='Audit/performance-wavelet-matrix-packed-2026-10-31.json';x=load(p);rs=[]
for e in x['parent_performance_summary']:
 c=e['condition']
 if c[1:] == ['api',1000000,0,31,'build',False]:rs.append(rec(c[0],False,e['ratio']['volatile'],e['baseline_process_cpu_ns'],e['baseline_process_cpu_ns']*e['ratio']['volatile'],{'parent_performance_summary.condition':c,'candidate':'volatile','metric':'whole childCPU ns'}))
for compiler in ['gcc','clang']:
 es=[e for e in x['native_long_summary']if e['compiler']==compiler];rs.append({'compiler':compiler,'native':True,'after_over_before_range':[min(e['ratio']for e in es),max(e['ratio']for e in es)],'raw_selected_conditions':es,'metric':'query body threadCPU','workload':'B64,n100000/500000,access/count/kth各200000照会','scope':'native final-wrapper object同一性bridge; default構築値とは異なるAPI条件'})
add('Wavelet Matrix','既定:31bit n1000000構築。native:B64 n100000/500000、3照会API各200000回','既定は子CPU全体、nativeは照会body','旧通常版→packed構築の最終volatile判定候補',rs,'構築0.182 / 照会0.999–1.014','構築0.266 / 照会0.974–1.010','既定/nativeで代表指標が違う。短いB64では一時+21%とcontrol変動があり長い測定へ。',[p],'native構築の最終候補再測定値はこの表に補間しない。native照会はほぼ中立。')
# 4
p='Audit/performance-linear-recurrence-weights-2026-10-31.json';x=load(p);rs=[]
for e in x['summary']:
 if e['condition'][2:]==[256,18446744073709551615,1]:rs.append(rec(e['condition'][0],e['condition'][1],e['ratios']['clang_weights_v2'],e['baseline_cpu_ns'],e['baseline_cpu_ns']*e['ratios']['clang_weights_v2'],{'condition':e['condition'],'candidate':'clang_weights_v2'}))
add('LinearRecurrenceWeights','uint32係数mod998244353、次数256、k=UINT64_MAX','body threadCPU ns / 照会','元のcanonical Weights→初期範囲shortcut+Clang Mont',rs,'1.001 / 1.000（専用対象外）','0.757 / 0.761','空入力に微小な負荷増。BM/Term/Prefix全体を高速化した数値ではない。',[p],'大きなkの通常照会を代表にし、k<次数で単位vectorになる最大改善は選んでいない。')
# 5
p='Audit/data/matrix-u32-tiled-final-2026-10-31-summary.json';x=load(p)['rows'];rs=[]
for e in x:
 if e['family']=='stress'and e['case']==[16,1024,1024]:
  m=e['metrics']['thread_us'];rs.append({**rec(e['compiler'],e['native'],m['ratios_baseline']['tile32_rows4'],m['medians']['baseline'],m['medians']['tile32_rows4'],{'family':'stress','case':[16,1024,1024],'candidate':'tile32_rows4','metric':'thread_us'}),'dedicated_kernel_applies':e['compiler']=='gcc'})
add('Matrix<uint32_t>','16×1024 と1024×1024 の積、unsigned mod2^32','body threadCPU μs / 積（同じ積5回の平均）','元i-k-j通常版→GCC tiled4row kernel',rs,'0.197 / 0.190','該当外（同一code）','旧full-stride候補の4096幅退行は不採用。小差・子wallの妨害をCPU値と区別。',[p,'Audit/performance-matrix-u32-tiled-2026-10-31.json'],'Clangの観測比0.950/0.983は同じ機械語の変動で、改善として表示しない。5標本中央値。')
# 6 two independent before baselines explicitly kept.
rs=[]
for file,candidate,compiler_only in [('Audit/data/modular-montgomery-2026-10-31/final_gcc_bench_summary.json','ready_final','gcc'),('Audit/data/modular-montgomery-2026-10-31/outlined_bench_summary.json','mont_subtract16_hybrid','clang')]:
 for e in load(file):
  if e['condition'][0]==compiler_only and e['condition'][2:]==[0,2,12]:rs.append({**rec(e['condition'][0],e['condition'][1],e['ratios_to_C'][candidate]['thread_us'],e['medians']['baseline_C']['thread_us'],e['medians'][candidate]['thread_us'],{'audit':file,'condition':e['condition'],'candidate':candidate}),'api':'ModPow','calls':16384})
p='Audit/benchmark-modular-sqrt-2026-10-31.json'
for e in load(p)['conditions']:
 if e['case']['shape']==5 and e['case']['pattern']==0:rs.append({**rec(e['compiler'],e['native'],e['selected_ratio']['thread_us'],e['baseline_us']['thread_us'],e['selected_us']['thread_us'],{'shape':5,'pattern':0,'prime':18446744069414584321}),'api':'ModSqrt','calls':8192})
add('ModPow / ModSqrt','Pow:最大64bit素数・乱数指数16384件。Sqrt:Goldilocks法の平方剰余8192件','body threadCPU μs / 記載件数','PowはC版e5a9…、SqrtはMont版9bd3…が各before（別基準）',rs,'Pow0.608/0.618、sqrt0.736/0.731','Pow0.609/0.636、sqrt0.742/0.724','Pow指数1で+.94ns、Clang p17 sqrtで+.5–1.3ns。先行Powの全条件を最終Sqrt後に再測定していない。',['Audit/benchmark-modular-montgomery-2026-10-31.json',p],'最終Sqrtの4Pow controlsとsource/機械語bridgeを別記録。2段階の比を掛けて推定しない。')
# 7
p='Audit/performance-fast-node-2026-10-31.json';x=load(p);rs=[]
for e in x['summary']:
 if e['n']==1000000 and e['mode']==0:
  c=e['compiler'];rs.append(rec('gcc'if c.startswith('gcc')else'clang',c.endswith('native'),e['ratios']['pool']['total_cpu_ns'],e['median']['baseline']['total_cpu_ns'],e['median']['pool']['total_cpu_ns'],{'n':1000000,'mode':0,'objects':1,'operations':2,'candidate':'pool'}))
add('Fast Node','1000000ノード構築+chain_size2回+破棄、mode0','phase合計threadCPU ns（OS終了回収を除く）','通常shared_ptr Node→Fast連続領域/pool Node',rs,'0.124 / 0.126','0.123 / 0.122','生ポインタAPI・共有所有なし。旧new-per-nodeのpush/pop退行をpoolで回避。',[p],'最大改善modeを選ばず単純な大きい列の走査を代表にした。')
# 8
p='Audit/performance-fast-binary-trie-2026-10-31.json';x=load(p);rs=[]
for e in x['summary']:
 if e['n']==200000 and e['mode']==8:
  c=e['compiler'];rs.append(rec('gcc'if c.startswith('gcc')else'clang',c.endswith('native'),e['ratios']['fast']['total_cpu_ns'],e['median']['baseline']['total_cpu_ns'],e['median']['fast']['total_cpu_ns'],{'n':200000,'mode':8,'operations':131072,'candidate':'fast'}))
add('Fast BinaryTrie（既定）','uint32/MAX_LOG29、131072件の混合操作、n条件200000','phase合計threadCPU ns（OS終了回収を除く）','通常BinaryTrie→Fast既定のlazy-ID',rs,'0.912 / 0.899','1.029 / 0.907','Clang既定はこの代表条件+2.9%。初回ID操作単体の退行も残る。inline-IDは別オプション。',[p,'Audit/fast-storage-adoption-2026-10-31.json'],'通常版の圧縮方式変更ではなく所有/ID保持方式の比較。')
# 9 use complete10large workloads median, not fastest one.
p='Audit/performance-fast-hash-release-2026-10-31.json';x=load(p);rs=[]
for e in x['selected_summary']:
 if e['candidate']=='BR'and' nosimd'not in e['mode']and'-nosimd'not in e['mode']:
  c=e['mode'];rs.append({**rec('gcc'if c.startswith('GCC')else'clang','native'in c,e['process_cpu_ratio_median'],selector={'mode':c,'candidate':'BR','filter':'index>=100 andn>64','aggregate':'median10large lifecycle ratios'}),'count':10})
add('Fast HashMap / HashSet','10個のlifecycle条件、n32768/262144、reserve/成長→追加→照会→破棄','子CPU全体（user+system）比の10条件中央値','通常汎用Hash→BR raw union slots、古い再配置領域は解放',rs,'0.804 / 0.801','0.802 / 0.798','列挙/空予約だけ等の後退はある。空予約だけの25–35倍を全体の効果としない。',[p],'Map/Setとキー型をまたぐ記録済み10条件の中央値。SIMD無効の追加4設定は元Auditに別記録。')
# 10 all6actual selectedFast workloads, no cherry-picked maximum per compiler.
p='Audit/fast-unreleased-vector-2026-10-31.json';x=load(p);rs=[];kinds=['fenwick_tree','segment_tree','lazy_segment_tree','wavelet_kth','implicit_treap_affine','aho_corasick_counts']
for e in x['all_case_details']:
 if e['metric']=='process_cpu_ns'and e['benchmark']=='verify'and e['size']=='large'and e['kind']in kinds:
  c=e['config'];selected='unreleased'if e['kind']=='implicit_treap_affine'or(e['kind']=='aho_corasick_counts'and c.startswith('gcc'))else'buffer';m=e['medians'];rs.append({**rec('gcc'if c.startswith('gcc')else'clang',c.endswith('native'),m[selected]/m['normal'],m['normal'],m[selected],{'benchmark':'verify','kind':e['kind'],'size':'large','metric':'process_cpu_ns','config':c,'selected':selected}),'library':e['kind']})
add('他Fast6種（F/S/L/W/T/A）','Fenwick/Segment/Lazy/Wavelet/Treap:n100000,Q200000。Aho:text500000,patterns50000','子CPU全体ns、各自の通常版をbeforeとした6種の範囲','同一Verify main/IOの通常版→各採用Fast版',rs,'0.965–1.019 / 0.961–1.010','0.831–1.029 / 0.846–1.021','大半は中立。Treapの追加利益は約1%、AhoはGCCだけUnreleased採用しClangの+17–20%切替は不採用。',[p,'Audit/fast-unreleased-vector-adoption-bridge-2026-10-31.json'],'F/S/L/WはFastBuffer、Tは両コンパイラUnreleased、AはGCC Unreleased/Clang FastBuffer。6種×4設定の全24記録をJSONに保存。')
assert len(rows)==10
out={'at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'new_measurements':False,'formal_files_changed':False,'cpu':'IntelCorei9-14900K P-CoreCPU12; collaborators builds/tests paused for original performance windows','shared_flags':'GCC15.2/Clang21.1.8 C++23 -O2 SKIP_RUNTIME_CHECKS; nativeadds-march=native','ratio_convention':'after/before elapsedCPU;1 is equal,smaller faster. D/N aredefault/native. Different baselines/metrics are explicitly separate; no multiplication or interpolation.','selection':'Fixed representative large inputs or fullrecorded4-setting workload range/median; no best-speed-only selection.','scope_caveats':['No onlineAC/onlinefastest claim.','Fast lifecycle/ownership andNodeAPI differ from ordinarysafeownership; read Fast contract before choice.','body andphase-total excludeprocessstartup/OSexit; whole-child includesuser+system. Walltime maydiffer.','FPS hasno separate numericperformance measurement; inheritedNTT only. MatrixClang isnotdedicatedtarget.','Normaloptimizations are already in bothnormal andFast sources; priorbaseline ratios are not a direct normal-vsFast comparison except explicitlyFast rows.'],'rows':rows,'audit_sha256':{p:hashlib.sha256((R/p).read_bytes()).hexdigest()for p in sorted({p for row in rows for p in row['audits']})}}
(O/'table.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n')
lines=['# 版を選ぶための代表速度表','','比率は **after / before**（小さいほど速い）。各セルの順は **既定 / native**。Intel Core i9-14900KのCPU12（P-Core）、GCC15.2／Clang21.1.8、C++23・O2・SKIP_RUNTIME_CHECKSでの既存記録。nativeだけ`-march=native`を追加した。通常版への今回の高速化はFastにも継承されている。','','| 対象 | 代表条件・測定範囲 | GCC 既定 / native | Clang 既定 / native | 主な後退・注意 |','| --- | --- | --- | --- | --- |']
for row in rows:lines.append('| '+row['name']+' | '+row['representative_workload']+'。'+row['metric_and_unit']+' | '+row['table_gcc_default_native']+' | '+row['table_clang_default_native']+' | '+row['major_regression_or_limit']+' |')
lines+=['','Fastのphase合計は構築・操作・破棄のCPU時間の和で、OS終了時の漏洩領域回収は含まない。子CPU全体は起動から終了までのuser+system。異なる測定範囲を横に比較しない。Fastは破棄・解放・例外保証を省く契約があり、通常版をそのまま置換できないAPIもある。','','PSTの通常/Fastはともに同じ旧通常版に対する比。ModPowのbeforeはC版、SqrtのbeforeはMontgomery導入後の版で別基準。Waveletの既定は構築、nativeは長い照会群で、未測定のnative構築値を補間していない。Fast6種の最後の行は全6種の範囲であり、各種の具体値はJSONにある。','','元Auditのキー・入力長・before/after・各4設定の値は [table.json](table.json)。この表は再測定・オンライン判定・すべての入力での優位を意味しない。']
(O/'table.md').write_text('\n'.join(lines)+'\n');print('TABLE_READY',len(rows),'rows')
