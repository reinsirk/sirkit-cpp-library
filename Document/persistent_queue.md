# 永続キュー

先頭を取り出したり末尾に追加したりしながら、過去の状態も保持できるキューです。版 `Version` を保存しておけば、同じ版から何通りもの操作を試せます。

`Version` は版を表す値型です。空の版は `Version{}` でも作れます。キューをコピーすると節点の保存領域を共有します。非空の版と要素への参照は、その保存領域を所有するキューが残る間有効です。非空の版はそれを作った保存領域を共有するキューに渡します。並行して更新・照会する場合は外部で同期します。

要素型 `T` はコピーまたはムーブで構築できる型を使います。累計の追加数 $P$ は $2^{32}-1$ 以下です。保存領域は $O(P+1)$ です。

## `PersistentQueue`

```text
template <class T>
class PersistentQueue;

PersistentQueue();
```

空のキューを作ります。 $O(1)$ です。

## `empty_version`

```text
static Version empty_version() noexcept;
```

どのキューにも渡せる空の版を返します。 $O(1)$ です。

## `push`

```text
Version push(Version version, T value);
```

指定版の末尾に値を追加した版を返します。元の版はそのまま使えます。 償却 $O(1)$ 時間・1節点です。

## `pop`

```text
Version pop(Version version) const;
```

空でない版の先頭を除いた版を返します。元の版はそのまま使えます。 最悪 $O(1+\log(P+1))$ 時間で節点を追加しません。

## `front`

```text
const T &front(Version version) const;
```

空でない版の先頭要素を読み取り専用参照で返します。 $O(1)$ です。

## `size`

```text
std::size_t size(Version version) const;
```

指定版の要素数を返します。 $O(1)$ です。

## `empty`

```text
bool empty(Version version) const;
```

指定版が空なら `true` を返します。 $O(1)$ です。

## 使用例

```cpp
sirkit::PersistentQueue<int> queue;
const auto one = queue.push(queue.empty_version(), 5);
const auto two = queue.push(one, 8);
const auto branch = queue.push(one, 13);
const auto rest = queue.pop(two);
const int first = queue.front(rest);        // 8
const int branch_head = queue.front(branch); // 5
```

[永続配列](persistent_array.md) は固定長で、添字を指定して値を読み書きできます。

[使用例](../Examples/persistent_queue_example.cpp) ／ [Verify](../Verify/current/submissions/persistent_queue.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/persistent_queue)
