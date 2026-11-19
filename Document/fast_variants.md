# 安全版と速度優先版

安全版は入力検査が既定で有効で、貼り付け前の `#define SKIP_RUNTIME_CHECKS` で対象の検査を省けます。速度優先版は、この条件を生成時に適用します。11ファイルにはメモリ管理も変更した専用実装があります。速度の差はライブラリ・入力・処理系によって変わり、安全版と同程度か、条件によって遅くなることもあります。

| 対応コンパイラ | 安全版 | 速度優先版 |
| --- | --- | --- |
| 共通 | [Variants/Portable](../Variants/Portable) | [Fast/Portable](../Fast/Portable) |
| GCC専用 | [Variants/GCC](../Variants/GCC) | [Fast/GCC](../Fast/GCC) |
| Clang専用 | [Variants/Clang](../Variants/Clang) | [Fast/Clang](../Fast/Clang) |
| MSVC専用 | [Variants/MSVC](../Variants/MSVC) | [Fast/MSVC](../Fast/MSVC) |

共通版はGCC・Clang・MSVCを対象に、処理系による分岐を保持します。専用版は対象の処理系で成立しない分岐を取り除き、CPU・コンパイラのバージョン・利用側の設定に依存する条件を保持します。MSVCではGNU系の128ビット整数やSIMD拡張に対する標準C++の代替処理を使います。

同じライブラリはプログラム全体で一つの版と設定にそろえます。`Algorithm`・`DataStructure`などにあるファイルが原本です。

## 速度優先版で残る検査

ライブラリ本体の配布ファイルでは、`SKIP_RUNTIME_CHECKS` を定義済みとして条件分岐を生成し、対象の入力検査と検査専用の状態を取り除きます。専用実装がないライブラリは、同じコンパイラ向けの安全版でこのマクロを定義した場合と同じ処理を使います。配布ファイル自体はこのマクロを定義しないので、同じ翻訳単位に貼る安全版は利用者側のマクロ設定に従います。

速度優先版の `temp.cpp` と `temp2.cpp` には `#define SKIP_RUNTIME_CHECKS` があり、貼り付け後のコードにも効きます。安全版のテンプレートでは、この既定の定義を取り除いています。速度優先版の単独提出用Verifyソースにも、このマクロを付けています。

検査の範囲は実装ごとに異なります。たとえば Fraction のゼロ除算や結果の表現範囲、Dijkstra の有限距離と無限大を表す値の衝突、NTT の対応する変換長などの保護と例外は速度優先版にも残ります。二部マッチングの処理量の上限も残ります。BinaryTrieの専用実装では、仮想ノード数の管理と容量検査も省きます。

## Fast版で異なる所有の契約

| 対象 | 領域・破棄の扱い |
| --- | --- |
| BinaryTrie | ノード・ID用ブロックとID配列の領域は破棄時も解放しない。削除した枠は内部で再利用する。コピー・ムーブ不可。 |
| Node | shared_ptrではなくNode*。共通プールや連続領域から確保し、個別には解放しない。 |
| Fenwick・Segment・Lazy Segment Tree、Wavelet Matrix | 主要配列に `FastBuffer` を使う。成長時の旧領域と最後の領域で、格納値の破棄と領域の解放を省く。clear・縮小は論理件数だけを減らす。 |
| Implicit Treap | ノード配列に `UnreleasedVector` を使う。成長・代入・clearは通常のvectorと同じ。オブジェクトの破棄では最後の領域と格納値の破棄を省く。 |
| Aho–Corasick | GCC専用版と共通版をGCCで使う場合は、対象配列に `UnreleasedVector` を使う。Clang・MSVC専用版と共通版をClang・MSVCで使う場合は `FastBuffer` を使う。各方式の解放の扱いは上の2行と同じ。 |
| Persistent Segment Tree | 状態本体を解放しない。reset・clear・上書き代入では旧状態を残す。各ノードvectorの成長時は旧領域を回収する。 |
| HashMap・HashSet | eraseでは値を破棄する。clearは配列を再利用し、表の破棄では最後の配列を解放しない。どちらも登録値の破棄を省く。再配置では旧配列を解放し、移動・コピー後の旧値のデストラクタは省く。 |

これらの保持領域はプロセス終了まで累積します。専用実装の対象外のライブラリや配列では、通常版の所有方式を維持します。格納値のデストラクタによる副作用や資源回収を必要とする用途、何度も作り直してメモリを回収したい用途には安全版を使います。

専用実装は有効な入力と確保・構築・演算の成功を利用条件としています。確保や利用者が渡した関数から例外が送出されることはあります。復元処理の有無も実装ごとに異なり、たとえばPersistent Segment Treeには、構築・更新の途中で例外が起きたときに追加したノードを戻す処理が残ります。

## 型・容量の注意

BinaryTrieではキー・XORマスク・順位・空集合への照会の前提を守ります。NODESは実ノード数とID枠数それぞれの確保上限で、通常版の仮想二分ノード数の管理・上限検査は行いません。各件数をsize_typeへ収めることも利用側の責任です。`SIRKIT_FAST_TRIE_INLINE_IDS`を貼り付け前に定義すると最大32個のID枠を内蔵しますが、IDなしの用途でもオブジェクトが大きくなります。

Node::nも生ポインタなので、通常版の.get()・.reset()は参照やnullptrの代入に置き換えます。clear_chainは先頭をnullptrにするだけです。プールは単一スレッド専用で、取得済みのアドレスを保ちます。循環の検出・切断は使えますが、長さ・末尾・複製・逆転は循環なしが前提です。

コピー可能な表・配列のコピーは独立した内容を持ちます。Implicit Treap・Aho–Corasick・HashMap・HashSetのmove元は自己ムーブを除き空で再利用可能です。Persistent Segment Treeのmove元へ行える操作は、通常版と同じく破棄・代入だけです。その他の前提は各ソース冒頭も確認してください。

## 生成と検証

```sh
python3 Scripts/generate_fast_sources.py
python3 Scripts/generate_variants.py
python3 Scripts/generate_fast_sources.py --check
python3 Scripts/generate_variants.py --check
```

専用生成元はFast/Source、対応表はScripts/fast_overrides.jsonです。生成済みファイルを直接編集せず、通常ソースまたは専用生成器を変更します。複数ライブラリの連結は[結合ツール](bundling.md)を使えます。

各PRで全8版の単独ビルドと動作を検査します。実行条件は[CIの品質検査](ci_quality.md)を参照してください。
