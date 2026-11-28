# 永続スタック

過去の状態を残しながら、任意の版の末尾に値を追加できます。版を表す `version_type` は `std::size_t` です。空の版は `PersistentStack<T>::kEmpty` です。操作に渡す版は同じオブジェクトで取得したものを使います。検査有効時は存在しない版で `std::out_of_range` を送出します。

`T` がコピー可能ならスタックもコピーできます。代入後は代入先の以前の版と参照を使えません。

## emplace

```text
template<class... Args> version_type emplace(version_type version, Args &&...args);
```

指定した版の末尾に引数から値を構築し、新しい版を返します。元の版はそのまま使えます。償却 `O(1)` 時間です。

## push

```text
version_type push(version_type version, const T &value);
version_type push(version_type version, T &&value);
```

値をコピーまたはムーブして末尾に追加します。償却 `O(1)` 時間です。

## pop

```text
version_type pop(version_type version) const;
```

末尾を除いた版を返します。空なら `kEmpty` を返します。`O(1)` 時間です。

## top

```text
const T &top(version_type version) const;
```

末尾の値を参照します。版は非空が前提です。検査有効時は空なら `std::out_of_range` を送出します。以降の追加でも参照は有効です。`O(1)` 時間です。

## 要素数

- `size(version)` は指定した版の要素数を返します。`O(1)` 時間です。
- `empty(version)` は指定した版が空かを判定します。`O(1)` 時間です。
- `node_count()` はオブジェクト全体で保存している追加ノード数です。`O(1)` 時間です。

空での構築は `O(1)` 時間です。追加回数を `N` として使用領域は `O(N+1)` です。

## 使用例

```cpp
sirkit::PersistentStack<int> stack;
const auto first = stack.push(stack.kEmpty, 3);
const auto a = stack.push(first, 7);
const auto b = stack.push(first, 9);
assert(stack.top(a) == 7 && stack.top(b) == 9);
assert(stack.pop(b) == first);
```

[完全な使用例](../Examples/persistent_stack_example.cpp)
