# Segment Tree Beats

`SegmentTreeBeats`は、整数列への区間chmin・chmax・加算と、区間和・最小値・最大値を扱います。chminは各要素を上限以下へ、chmaxは下限以上へそろえる操作です。添字は0始まり、区間は半開区間[l,r)です。[使用例](../Examples/segment_tree_beats_example.cpp)を参照してください。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
SegmentTreeBeats() = default;
explicit SegmentTreeBeats(const std::vector<std::int64_t>& values);
SegmentTreeBeats(const SegmentTreeBeats&) = default;
SegmentTreeBeats(SegmentTreeBeats&& other) noexcept;
```

空列または指定したint64_t列で構築します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素にO(N+1)時間・領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## chmin

```text
void chmin(std::size_t l, std::size_t r, std::int64_t x);
```

区間の各値をmin(現在値,x)へ更新します。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** 1回は最悪O(N+1)。Q回の範囲更新は合計O((N+Q) log²(N+1))の償却上界。

## chmax

```text
void chmax(std::size_t l, std::size_t r, std::int64_t x);
```

区間の各値をmax(現在値,x)へ更新します。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** 1回は最悪O(N+1)。Q回の範囲更新は合計O((N+Q) log²(N+1))の償却上界。

## add

```text
void add(std::size_t l, std::size_t r, std::int64_t x);
```

区間の各値へxを加算します。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** 1回は最悪O(N+1)。Q回の範囲更新は合計O((N+Q) log²(N+1))の償却上界。

## set

```text
void set(std::size_t i, std::int64_t value);
```

指定位置の値を置き換えます。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。

## get

```text
std::int64_t get(std::size_t i);
```

指定位置の値を取得します。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。

## sum

```text
std::int64_t sum(std::size_t l, std::size_t r);
```

区間和をint64_tで返します。空区間は0、結果が範囲外ならoverflow_errorです。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。

## min

```text
std::optional<std::int64_t> min(std::size_t l, std::size_t r);
```

区間最小値をoptional<int64_t>で返します。空区間はnulloptです。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。

## max

```text
std::optional<std::int64_t> max(std::size_t l, std::size_t r);
```

区間最大値をoptional<int64_t>で返します。空区間はnulloptです。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。

## values

```text
std::vector<std::int64_t> values();
```

現在の列をvectorへコピーして返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(N+1)時間・O(N)追加領域。

## size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## reset

```text
void reset(const std::vector<std::int64_t>& values);
```

指定列で再構築します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+新N+1)時間・新列の保持領域。

## clear

```text
void clear() noexcept;
```

空の状態へ戻します。容量や資源の扱いは末尾の所有・例外の説明に従います。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+1)の上界。要素・設定の破棄費用を含みます。

## swap

```text
void swap(SegmentTreeBeats& other) noexcept;
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
SegmentTreeBeats tree(std::vector<std::int64_t>{1,5,-2,8});
tree.chmin(0,4,4); // [1,4,-2,4]。
tree.chmax(1,3,0); // [1,4,0,4]。
tree.add(0,2,3);   // [4,7,0,4]。
auto sum = tree.sum(0,4); // 15。
```

## 前提と計算量

最大値・2番目の最大値・個数などを持ち、影響する要素だけをまとめて更新できる区間では子へ降りません。構築はO(N+1)時間・領域。N要素にQ回の範囲更新を行う全体の償却計算量はO((N+Q) log²(N+1))、1回の範囲更新は最悪O(N+1)です。set・get・sum・min・maxはO(log(N+1))です。

要素にはint64_tの全値域を使えます。add後の実際の要素が範囲外ならoverflow_errorを投げて列を保ちます。内部の遅延配置は変わる場合があります。sumの戻り値が範囲外の場合もoverflow_errorです。和だけが大きい列は保持・更新でき、極値や別区間を取得できます。空区間の更新は何もしません。

内部の和と遅延はGCC/Clangの128ビット整数拡張、または標準C++の2ワード演算を使います。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準経路へ切り替えます。標準経路はsize_tが64ビット以下の環境が対象です。中間の子の値だけが64ビット範囲外でも扱えるよう計算し、容量・桁あふれの保護は検査省略時も維持します。数値設計の詳細は[Audit](../Audit/)に置きます。

取得も保留中の更新を子へ伝えるため非constです。更新・区間取得は動的確保を行わず、valuesは戻す配列を確保します。reset・コピー代入は失敗時に元を保ち、コピーは独立、移動元は空として再利用できます。resetはO(旧N+新N+1)、clearはO(N+1)で、確保容量を保持する場合があります。

添字・区間は既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。構成と問題選定は[NyaanのSegment Tree Beats](https://nyaannyaan.github.io/library/segment-tree/segment-tree-beats.hpp)を参考にし、実装は独立に組み立てています。

## operator=

```text
SegmentTreeBeats& operator=(const SegmentTreeBeats& other);
SegmentTreeBeats& operator=(SegmentTreeBeats&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
