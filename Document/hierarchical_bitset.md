# 階層ビット集合

$0$ から $n-1$ までの整数を集合に出し入れし、指定値以上の最小要素や指定値以下の最大要素を求めます。64ビット語ごとの有無を上位のビットで持つため、空の区間を飛ばして探せます。

添字空間の長さを $n$ とします。追加・削除・所属判定の添字は `position < n` を満たしてください。入力検査が有効な場合、範囲外の添字には `std::out_of_range` を送出します。

## `HierarchicalBitset`

```text
explicit HierarchicalBitset(std::size_t n = 0);
explicit HierarchicalBitset(std::string_view bits);
```

長さ `n` を渡すと空集合を作ります。文字列を渡すと各 `'1'` の位置を初期要素にします。文字列は `'0'` と `'1'` で構成してください。入力検査が有効な場合、それ以外の文字には `std::invalid_argument` を送出します。空集合の構築は $O(1+n/64)$ 時間、文字列からの構築は $O(n+1)$ 時間です。保存領域はいずれも $O(1+n/64)$ ワードです。

## `insert`

```text
bool insert(std::size_t position);
```

要素を追加し、新しく加わった場合に `true` を返します。$O(1+\log_{64}(n+1))$ です。

## `erase`

```text
bool erase(std::size_t position);
```

要素を削除し、削除できた場合に `true` を返します。$O(1+\log_{64}(n+1))$ です。

## `contains`

```text
bool contains(std::size_t position) const;
```

要素を含むかを $O(1)$ で返します。

## `next`

```text
std::optional<std::size_t> next(std::size_t from) const noexcept;
```

`from` 以上の最小要素を返します。該当要素がなければ `std::nullopt` です。$O(1+\log_{64}(n+1))$ です。

## `prev`

```text
std::optional<std::size_t> prev(std::size_t from) const noexcept;
```

`from` 以下の最大要素を返します。該当要素がなければ `std::nullopt` です。`from >= n` のときも呼び出せます。$O(1+\log_{64}(n+1))$ です。

## `size`

```text
std::size_t size() const noexcept;
```

添字空間の長さを $O(1)$ で返します。

## `count`

```text
std::size_t count() const noexcept;
```

集合の要素数を $O(1)$ で返します。

## `empty`

```text
bool empty() const noexcept;
```

要素がなければ `true` を返します。$O(1)$ です。

## 使用例

```cpp
sirkit::HierarchicalBitset set("010010");
set.next(1); // 1
set.prev(4); // 4
set.next(2); // 4
set.insert(3);
set.erase(4);
set.next(2); // 3
```

[Dynamic Bitset](dynamic_bitset.md) はビット演算、[Fenwick Tree](fenwick_tree.md) は区間の個数や順位、[Segment Tree](segment_tree.md) は自由な演算での集約と境界探索に使えます。

[使用例](../Examples/hierarchical_bitset_example.cpp) ／ [Verify](../Verify/current/submissions/predecessor_problem.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/predecessor_problem)
