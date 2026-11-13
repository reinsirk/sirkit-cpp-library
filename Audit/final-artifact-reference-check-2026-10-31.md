# 不変artifact参照の読み取りQA

今回の日付を含むAudit JSON **202本**を走査し、SHA付き保存物参照 **248件**、実ファイル **180個**を照合した。存在・記載SHA・サイズの実不整合は **0件**。サイズが記載された参照は204件、SHAだけの参照は44件で、サイズ未記載を一致扱いへ読み替えていない。

対象は`Audit/data/...`の不変保存物だけ。`/tmp`の測定ソース、過去版のSHA、現行C++ソースとの一致は調べていない。baseline ZIPに無いJSONは846本だったが、baseline自体が以前のAuditを省いていたため、古い日付の644本を今回の新設と誤認しないよう除外した。この選択範囲を超える保証はしない。

認識したschema:

- パスをキーにしたSHA、または`{sha256,bytes}`。
- 同じrecord内の明示的な保存物パスと`sha256`。
- 名前で対応したパス・`*_sha256`の組（今回の実データでは0件、認識probeで確認）。
- `Audit/data/**/manifest.json`の`files`辞書にある相対ファイル名・SHA・サイズ。

SHAと同一recordで関連付けられない8個のリンクは対象外として明記した。任意の相対パス、opaqueな埋込文字列、ZIP/gzip内部の意味、アルゴリズムの正しさ、コンパイル済みbinaryの性質は検査範囲外。

CPU4で逐次実行し、ファイルhashは1MiBずつ読む。実ファイル合計47,234,768バイト、最大artifactは14,460,837バイト。入力JSONのstat変更は検出されず、parseエラーもない。schema認識のprobe7条件も通過した。

詳細な各参照・実SHA・実サイズ・対象外schema・入力JSONのSHAは[検査結果](final-artifact-reference-check-2026-10-31.json)、読み取り専用スクリプトは[こちら](data/final-artifact-reference-check-2026-10-31.py)。パス・入力選択・SHAは記録時点のものです。
