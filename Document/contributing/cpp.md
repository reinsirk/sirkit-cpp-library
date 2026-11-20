# C++の実装規約

## 公開APIと命名

- 標準C++23まで使用可。公開APIはライブラリ専用の`namespace sirkit`に置き、内部専用の関数・型は`sirkit::detail`に置く。既存の幾何APIの`geo`も`sirkit::geo`とする。ライブラリ内でusing-directiveは使わず、競プロ用mainでは`using namespace sirkit;`を使ってよい。関数名・型名には個人名の接頭辞を付けない。インクルードガードと設定マクロの接頭辞は`SIRKIT_`に統一する。`SKIP_RUNTIME_CHECKS`はこの名前を維持する。`long long`ではなく`std::int64_t`等を使う。
- Fenwick Tree・Segment Tree・Lazy Segment Treeは、`sirkit::fenwick_tree`・`sirkit::segtree`・`sirkit::lazy_segtree`として、名前空間以外はACLの型引数・引数型・戻り値・呼び出し形式に合わせる。追加機能はよいが、同じ機能を異なる名前の公開APIで重複させない。
- ACLに対応するAPIがあるライブラリは、`sirkit`内で型名・型引数・引数型・戻り値・呼び出し形式をACLに合わせる。最大流は`sirkit::mf_graph<Cap>`、辺は`edge { int from, to; Cap cap, flow; }`、辺番号は`int`とする。独自の追加機能は同じ機能の別名を増やさずに提供する。
- 定数の命名はGoogle式の`k`＋PascalCaseを基本とする。変更可能な変数に`k`は付けず、公開データはsnake_case、非公開データメンバーはsnake_case_とする。自動記憶域のconst変数は通常の変数名でもよい。ACL互換APIの型名・操作名はACL互換の方針を優先し、他の衝突は利用者へ報告する。
- 改修時は用途に合う便利な機能も検討して追加する。既存操作の速度・所有権・正しさを保ち、追加機能も先頭のAPI説明と使用例に含める。

## 所有権と入力検査

- `new`式を直接書くことは原則禁止。所有には`std::vector`や`std::make_unique`等を使い、参照用の非所有ポインタと区別する。
- 正しさ、安全な所有権管理、実行速度を重視する。計算量に加え、確保・初期化・分岐・メモリ配置も検討し、速度の主張は測定条件と結果に基づける。
- 必要な低コストの入力検査は既定で有効にする。`SKIP_RUNTIME_CHECKS`が定義されていれば、プリプロセッサで入力検査と検査専用の状態を除く。
- 検査のためだけに配列や全辺を走査しない。通常処理へ検査を組み込む場合も、繰り返し検査による負荷を抑える。未参照部分の入力まで検査できるとは説明しない。
- 内部の容量・添字オーバーフロー対策、計算予算の上限、数学的な0判定は検査を省く場合も維持する。

## 単独利用と共通部品

- 各ライブラリは標準ヘッダーを明示し、自作の別ファイルのincludeに依存せず単独で貼り付けられるようにする。Mintの実装は利用側が用意する。
- ファイル全体をインクルードガードで囲まない。includeは外に置き、関数・クラス・定数・共通部品ごとにガードを付ける。重み付きの`Edge`と`WGraph`は`SIRKIT_GRAPH_TYPES_DEFINED`、無重みの`Graph`は`SIRKIT_UNWEIGHTED_GRAPH_DEFINED`で共有し、`Edge`は`int to; std::int64_t w;`とする。`temp*`には両方を定義する。
- `kInf`はグラフ共通型のガードの外に配置し、専用の共通ガード`SIRKIT_INF_DEFINED`で共有する。`sirkit`内の定義は`inline constexpr std::int64_t kInf = std::int64_t{1} << 60;`に統一する。旧名`INF`の別名は増やさない。
- 軽量ハッシュ表は内部用`sirkit::detail::InternalHashMap`とし、`SIRKIT_INTERNAL_HASH_MAP_DEFINED`で共有する。公開の汎用版`HashMap`とそのガード`SIRKIT_HASH_MAP_DEFINED`を混同しない。汎用ハッシュ関数`GenericHash`にも独立ガードを付ける。
- 二項係数ライブラリの補助的な素数判定は、先頭にプロトタイプ宣言だけを置き、定義は本体の後に配置する。定義の共有ガードと検査省略時の除外を維持する。

## 処理系の拡張と代替実装

- GCC/Clangで128ビット整数拡張が利用可能なら高速な演算に使い、`#if`で標準C++の代替実装へ切り替える。拡張型を標準の型とは説明しない。
- 実測で効果があるSIMD高速化は積極的に取り入れる。専用実装には標準C++23の代替経路を用意し、コンパイラとCPUの対応を確認して選択する。`SIRKIT_DISABLE_SIMD`で追加したSIMD実装とCPU判定を除外する。小規模入力の切り替え費用と、自動ベクトル化済みの実装との差も測定する。
