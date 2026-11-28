# PersistentBinaryTrie

整数の多重集合を版ごとに保持します。接頭辞の版を順に作ると、添字区間内のk番目の値やXOR後の最小値を求められます。

```cpp
sirkit::PersistentBinaryTrie<unsigned, 9> trie(3);
const auto a = trie.insert({}, 2);
const auto b = trie.insert(a, 5);
const auto c = trie.insert(b, 3);
trie.kth_xor(a, c, 0);    // {5, 3} の最小値 3
trie.kth_xor(a, c, 0, 6); // {5 ^ 6, 3 ^ 6} の最小値 3
trie.size(a); // 1
```

[使用例](../Examples/persistent_binary_trie_example.cpp)

## `PersistentBinaryTrie`

```text
PersistentBinaryTrie<T = std::uint32_t, MAX_LOG = std::numeric_limits<T>::digits - 1>
explicit PersistentBinaryTrie(std::size_t expected_updates = 0)
```

`T` は `bool` を除く符号なし整数型です。キーとXORマスクは `B = MAX_LOG + 1` ビット以内です。既定では `T` の全ビットを使います。構築時は見込みの更新回数に応じて領域を確保します。

空の版は `Version{}` で表します。更新後の版は同じ木で使います。木を破棄したり、別の木を代入したりすると、それまでの版は使えなくなります。

## `insert`

```text
Version insert(Version version, T key)
```

`key` を1個加えた版を返します。元の版も使えます。償却 `O(B)` 時間、追加領域 `O(B)` です。

## `erase`

```text
std::optional<Version> erase(Version version, T key)
```

`key` を1個除いた版を返します。キーがなければ `std::nullopt` を返します。償却 `O(B)` 時間、追加領域 `O(B)` です。

## `size` と `count`

```text
std::uint32_t size(Version version) const
std::uint32_t count(Version version, T key) const
```

版の要素数を `O(1)`、指定キーの個数を `O(B)` で返します。

## `kth_xor`

```text
std::optional<T> kth_xor(Version before, Version after, std::uint32_t k, T mask = 0) const
```

`after` から `before` の要素を引いた多重集合で、`key ^ mask` の昇順で0始まりの `k` 番目の値を返します。要素が足りなければ `std::nullopt` です。`before` は `after` の部分多重集合である必要があります。時間は `O(B)` です。

## `count_xor_less`

```text
std::uint32_t count_xor_less(Version before, Version after, T bound, T mask = 0) const
```

同じ差分で `key ^ mask < bound` を満たす要素数を `O(B)` で返します。`bound` がキーの最大値より大きければ差分の要素数を返します。
