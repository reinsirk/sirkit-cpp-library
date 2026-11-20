# Prüfer符号と指定次数の木

頂点番号0..N-1の木とPrüfer符号を相互変換します。最小番号の葉を順に除いたときの隣接頂点を記録する定義です。入力を変更せず、結果の配列を所有して返します。

## pruefer_encode

```text
inline std::vector<int> pruefer_encode(int n, std::span<const std::pair<int, int>> edges);
```

単純無向木の辺列からPrüfer符号を返します。辺の順序・向きは問いません。

**制約:** n>=1、辺はn-1本で端点は0..n-1。

**計算量:** O(n)時間・領域。

## pruefer_decode

```text
inline std::vector<std::pair<int, int>> pruefer_decode(int n, std::span<const int> code);
inline std::vector<std::pair<int, int>> pruefer_decode(std::span<const int> code);
```

Prüfer符号から木の辺列を返します。符号だけの版はn=code.size()+2とします。

**制約:** n>=1、符号長はmax(0,n-2)、各要素は0..n-1。1頂点はnを明示してください。

**計算量:** O(n)時間・領域。

## pruefer_degrees

```text
inline std::vector<int> pruefer_degrees(int n, std::span<const int> code);
```

符号に対応する各頂点の次数を返します。

**制約:** n>=1、符号長はmax(0,n-2)、各要素は0..n-1。

**計算量:** O(n)時間・領域。

## pruefer_from_degrees

```text
inline std::optional<std::vector<int>> pruefer_from_degrees(std::span<const int> degrees);
```

指定次数を持つ木のPrüfer符号を返します。実現不能ならnulloptです。

**制約:** n>=2では全次数>=1かつ総和2(n-1)。n=1は{0}のみ可、空列は不可です。

**計算量:** O(n)時間・領域。

## tree_from_degrees

```text
inline std::optional<std::vector<std::pair<int, int>>> tree_from_degrees(std::span<const int> degrees);
```

指定次数を持つ木の辺列を返します。実現不能ならnulloptです。

**制約:** 次数列の実現条件はpruefer_from_degreesと同じです。

**計算量:** O(n)時間・領域。

## complete_tree_with_degrees

```text
inline std::optional<std::vector<std::pair<int, int>>> complete_tree_with_degrees( std::span<const int> degrees, std::span<const std::pair<int, int>> edges);
```

既存の森の辺を残し、指定次数の木を完成させる追加辺だけを返します。実現不能ならnulloptです。

**制約:** 次数・頂点番号は有効範囲。循環、残余次数不足、接続不能な成分もnulloptです。

**計算量:** n頂点・m既存辺にO((n+m)α(n))時間・O(n+m)領域。

## 境界・検査

n=1では符号・辺が空で次数は{0}、n=2では符号が空、辺は{0,1}、次数は{1,1}です。

入力長・端点・符号値・処理中に分かる非木の検査は`SKIP_RUNTIME_CHECKS`で除きます。次数列・森の完成の実現可能性と件数上限は常に判定します。未参照入力まで検査する保証はありません。確保失敗時も入力は変更しません。

```cpp
std::vector<int> code{3, 4, 3, 5, 4};
auto edges = pruefer_decode(7, code);
auto restored = pruefer_encode(7, edges); // codeと一致。
auto degrees = pruefer_degrees(7, code); // {1,1,1,3,3,2,1}。
```

```cpp
auto tree = tree_from_degrees(std::vector<int>{1, 2, 1});
auto bad = tree_from_degrees(std::vector<int>{2, 2, 2}); // nullopt。
```

```cpp
std::vector<int> degrees{1, 2, 1, 2, 2, 2};
std::vector<std::pair<int,int>> forest{{1,2}, {0,3}};
auto added = complete_tree_with_degrees(degrees, forest);
```

[完全な使用例](../Examples/pruefer_code_example.cpp)・[数学的参考](https://info.atcoder.jp/entry/algorithm_lectures/prufer_code)
