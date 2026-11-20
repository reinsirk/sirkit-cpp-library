# コンテスト用テンプレート

[temp.cpp](../temp.cpp)にはACL全体のインクルードパスが必要です。[temp2.cpp](../temp2.cpp)は標準ライブラリだけで使えます。GCC系の一括ヘッダーを使える環境ではこれを読み込み、MSVCなどでは個別の標準ヘッダーを読み込みます。

原本のテンプレートと速度優先版では`SKIP_RUNTIME_CHECKS`を定義しています。配布用の通常版では検査が有効です。ライブラリを貼り付ける位置より前でこのマクロを定義すると検査を省けます。

| 型・関数 | 内容 |
| --- | --- |
| `Graph` | `std::vector<std::vector<int>>`。無重み隣接リスト。 |
| `Edge` / `WGraph` | 行先`int to`・重み`std::int64_t w`と、その隣接リスト。 |
| `kInf` | `std::int64_t{1} << 60`。 |
| `chmin(a,b)` / `chmax(a,b)` | 値を更新したらtrue。比較・代入各1回以下。 |
| `pow64i(x,n)` | x^n mod 2^64。O(log(n+1))時間。0^0は1。 |
| `is_mul_overflow(a,b)` | 積が符号付き64ビット整数の範囲外ならtrue。O(1)時間。 |

グラフ型と`kInf`は、ライブラリと同じガードを使って定義を共有します。`is_mul_overflow`はGCC/Clangの組込み演算を使い、`SIRKIT_FORCE_STANDARD_MUL_OVERFLOW`を定義すると標準C++の除算による判定へ切り替えます。
