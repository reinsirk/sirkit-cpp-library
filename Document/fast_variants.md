# コンパイラ別版と速度優先版

同じ公開APIで、コンパイラ別に不要な分岐を省いた版と、検査・領域管理を省く速度優先版を用意しています。通常版を基本とし、Fastの前提を満たす場合に切り替えてください。常に速くなる保証はありません。

| 配置 | 選び方・通常版との違い |
| --- | --- |
| `Algorithm`・`DataStructure`等 | 通常版。所有・検査の契約は各ライブラリの説明に従う。 |
| `Variants/GCC` / `Variants/Clang` | 対応コンパイラ向けに分岐を整理。通常版と同じ所有・検査。 |
| `Fast/GCC` / `Fast/Clang` | 入力検査と検査専用の状態を省く。一部は以下の所有・容量の契約も異なる。 |

同じライブラリはプログラム全体で一つの版・設定にそろえます。公開名とガードは共通ですが、Fastでは型や配置が変わる場合があります。生成対象はGNU互換モードのGCC・Clangです。CPU対応判定、SIRKIT_DISABLE_SIMD、128ビット拡張の有無、標準演算への切り替えは残し、CPU命令を無条件には使いません。

## Fast版で異なる所有の契約

| 対象 | 領域・破棄の扱い |
| --- | --- |
| BinaryTrie | ノード・ID用ブロックとID配列の領域は破棄時も解放しない。削除した枠は内部で再利用する。コピー・ムーブ不可。 |
| Node | shared_ptrではなくNode*。共通プールや連続領域から確保し、個別には解放しない。 |
| Fenwick・Segment・Lazy Segment Tree、Wavelet Matrix、Clang版Aho–Corasick | FastBufferが主要領域を保持。成長時の旧領域・格納値と最後の領域を破棄・解放しない。clear・縮小は論理件数だけを減らす。 |
| Implicit Treap、GCC版Aho–Corasick | UnreleasedVectorで通常の成長・代入・clearを行う。成長時は旧領域を回収し、clearは要素を破棄する。最後の領域・格納値の破棄だけを省く。 |
| Persistent Segment Tree | 状態本体を解放しない。reset・clearは新状態へ切り替え、旧状態を残す。各ノードvectorの成長時は旧領域を解放する。 |
| HashMap・HashSet | clear・表の破棄で登録値と最後の配列を破棄・解放しない。再配置では旧配列を解放するが、非自明な値をコピーして移す場合は旧値の資源回収を省く。 |

これらの保持領域はプロセス終了まで累積します。格納値のデストラクタによる副作用や資源回収を必要とする用途、何度も作り直してメモリを回収したい用途には通常版を使います。公開APIで受け渡すvectorや一部の作業配列まで、全て解放を省くわけではありません。

Fastは有効な入力と確保・構築・演算の成功を前提にします。例外後の状態復元は保証しません。検査を除いただけのライブラリは通常版の所有方式を維持し、利用側へSKIP_RUNTIME_CHECKSを定義する副作用もありません。

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

専用生成元はFast/Source、対応表はScripts/fast_overrides.jsonです。生成済みファイルを直接編集せず、通常ソースまたは専用生成器を変更します。複数ライブラリの連結は[結合ツール](bundling.md)を使えます。生成条件・内部の確保方式・測定結果は[Audit](../Audit/)に置きます。
