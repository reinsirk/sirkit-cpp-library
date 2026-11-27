# 編集距離

挿入・削除・置換の回数が最小になるように、一方の文字列をもう一方に変えます。

```cpp
assert(sirkit::edit_distance("kitten", "sitting") == 3);
assert(!sirkit::edit_distance_at_most("kitten", "sitting", 2));
```

## `edit_distance`

```text
std::size_t edit_distance(std::string_view a, std::string_view b);
```

1文字の挿入・削除・置換をそれぞれ1回と数え、最小回数を返します。長さを $n,m$ として $O(nm+1)$ 時間・$O(\min(n,m)+1)$ 領域です。

## `edit_distance_at_most`

```text
std::optional<std::size_t> edit_distance_at_most(
    std::string_view a, std::string_view b, std::size_t limit);
```

距離が `limit` 以下ならその距離、超えるなら `std::nullopt` を返します。上限を $k$ とすると、$k<\max(n,m)$ では $O((\max(n,m)+1)\min(2k+1,\min(n,m)+1))$ 時間・$O(\min(k,n,m)+1)$ 領域です。上限がそれ以上なら通常の編集距離を計算します。

[使用例](../Examples/edit_distance_example.cpp) ／ [Verify](../Verify/current/submissions/bounded_edit_distance.cpp)
