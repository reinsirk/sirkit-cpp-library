# Wavelet Matrix

`WaveletMatrix<T>`は、変更しない整数列に対して「区間内でk番目に小さい値」「指定値の個数」などを照会します。区間は0始まりの半開区間[l,r)、順位kも0始まりです。[使用例](../Examples/wavelet_matrix_example.cpp)を参照してください。

以下、Nは列長、Bは値の範囲に必要なbit幅（0〜64）です。

## コンストラクタ

```text
WaveletMatrix() = default;
explicit WaveletMatrix(std::span<const T> values);
explicit WaveletMatrix(const std::vector<T>& values);
WaveletMatrix(const WaveletMatrix&) = default;
WaveletMatrix(WaveletMatrix&& other) noexcept;
template <class T> WaveletMatrix(const std::vector<T>&) -> WaveletMatrix<T>;
template <class T> WaveletMatrix(std::span<const T>) -> WaveletMatrix<T>;
```

空列または指定したvector・spanから構築します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素・必要bit幅BにO(N(1+B))時間、O(1+B ceil(N/64))語の保持領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## build

```text
void build(std::span<const T> values);
void build(const std::vector<T>& values);
```

指定したvectorまたはspanから再構築します。

**制約:** Tはbool以外の64ビット以下の整数、列長はUINT32_MAX以下。

**計算量:** N要素・必要bit幅BにO(N(1+B))時間。構築中の追加領域はO(N)。

## access

```text
T access(std::size_t index) const;
```

添字iの元の値を返します。

**制約:** 0<=index<size()。

**計算量:** 必要bit幅BにO(1+B)。

## operator[]

```text
T operator[](std::size_t index) const;
```

accessと同じく添字iの元の値を返します。

**制約:** 0<=index<size()。

**計算量:** 必要bit幅BにO(1+B)。

## kth_smallest

```text
T kth_smallest(std::size_t left, std::size_t right, std::size_t k) const;
```

区間内の値を昇順に並べたとき、0始まりでk番目の値を返します。

**制約:** 0<=left<=right<=size()、0<=k<right-left。

**計算量:** 必要bit幅BにO(1+B)。

## kth_largest

```text
T kth_largest(std::size_t left, std::size_t right, std::size_t k) const;
```

区間内の値を降順に並べたとき、0始まりでk番目の値を返します。

**制約:** 0<=left<=right<=size()、0<=k<right-left。

**計算量:** 必要bit幅BにO(1+B)。

## range_min

```text
T range_min(std::size_t left, std::size_t right) const;
```

非空区間の最小値を返します。

**制約:** 0<=left<right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## range_max

```text
T range_max(std::size_t left, std::size_t right) const;
```

非空区間の最大値を返します。

**制約:** 0<=left<right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## count

```text
std::size_t count(std::size_t left, std::size_t right, T value) const;
```

半開区間内のxの個数を返します。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## rank

```text
std::size_t rank(T value, std::size_t left, std::size_t right) const;
```

半開区間内のxの個数を返します。countとは引数順が異なります。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## count_less

```text
std::size_t count_less(std::size_t left, std::size_t right, T value) const;
```

半開区間内のx未満の値の個数を返します。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## count_less_equal

```text
std::size_t count_less_equal(std::size_t left, std::size_t right, T value) const;
```

半開区間内のx以下の値の個数を返します。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## range_freq

```text
std::size_t range_freq(std::size_t left, std::size_t right, T lower, T upper) const;
```

半開区間内のlower以上upper未満の個数を返します。lower>=upperなら0です。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## predecessor

```text
std::optional<T> predecessor(std::size_t left, std::size_t right, T value) const;
```

区間内でx未満の最大値をoptionalで返します。なければnulloptです。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

## successor

```text
std::optional<T> successor(std::size_t left, std::size_t right, T value) const;
```

区間内でx以上の最小値をoptionalで返します。なければnulloptです。

**制約:** 0<=left<=right<=size()。

**計算量:** 必要bit幅BにO(1+B)。

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

## clear

```text
void clear() noexcept;
```

空列へ戻します。

**制約:** 共通の前提を満たすこと。

**計算量:** 保持領域の破棄に比例。

## swap

```text
void swap(WaveletMatrix& other) noexcept;
friend void swap(WaveletMatrix& a, WaveletMatrix& b) noexcept;
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
WaveletMatrix<std::int64_t> matrix(std::vector<std::int64_t>{-5,7,2,-5,10});
auto value = matrix.kth_smallest(0,5,1); // -5。小さい順で2番目。
auto count = matrix.count(0,5,-5);      // 2。
auto next = matrix.successor(0,5,3);    // optionalに7が入る。
```

## 前提と計算量

Tはbool以外の64ビット以下の整数型です。符号付き型の最小値・最大値も扱えます。長さはUINT32_MAX以下で、構築時に確認します。

値を順序が変わらない符号へ変換して最小値を引き、必要なビット幅B（0〜64）だけを扱います。構築はO(N(1+B))、照会はO(1+B)、格納はO(1+B ceil(N/64))語、構築中の追加領域はO(N)です。すべての値が同じならB=0となり、照会はO(1)です。

コピーは独立、buildとコピー代入は失敗時に元の列を維持します。ムーブ元は空列です。`SKIP_RUNTIME_CHECKS`で添字・区間・順位の検査を除きますが、長さの表現上限は維持します。

対応CPUではPOPCNT・AVX2を使います。`SIRKIT_DISABLE_SIMD`で追加の専用実装とCPU判定を除き、標準C++経路へ切り替えます。コンパイラ自身の自動ベクトル化は通常の設定に従います。経路の条件と測定は[Audit](../Audit/)を参照してください。

数学的参考は[AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/wavelet_matrix_basic)です。実装は本ライブラリ用に独立に組み立てています。

## operator=

```text
WaveletMatrix& operator=(const WaveletMatrix& other);
WaveletMatrix& operator=(WaveletMatrix&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
