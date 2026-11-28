# 削除できる整数集合の後継探索

最初は `0` から `n-1` がすべて入っています。値を取り除きながら、指定位置以上で最初に残っている値を探せます。

| API | 内容 |
| --- | --- |
| `SuccessorSet(n)` | 全要素を残した状態で構築する。$O(n+1)$ 時間・領域。 |
| `next(i)` | `i` 以上で最小の残存値。なければ `std::nullopt`。 |
| `erase(i)` / `take(i)` | `i` を削除して成功可否を返す / `next(i)` を取得して削除する。 |
| `take_cyclic(i)` | 末尾まで見つからなければ先頭へ戻って取得・削除する。 |

要素数 `size()` と残存数 `remaining()` は $O(1)$、その他の操作は償却 $O(\alpha(n+1))$ です。`i = n` は末尾の直後を表します。

```cpp
sirkit::SuccessorSet slots(4);
slots.erase(2);
auto next = slots.take(2); // 3
auto wrap = slots.take_cyclic(2); // 0
```

`parking_slots(preferences)` は希望位置から右へ探して、全員の実際の配置を返します。全員が入らなければ `std::nullopt` です。$n$ 人の配置に償却 $O(n\alpha(n+1))$ 時間かかります。

[使用例](../Examples/successor_set_example.cpp) ／ [Verify](../Verify/current/submissions/successor_set_linear_probing.cpp) ／ [Parking Functionの解説](https://info.atcoder.jp/entry/algorithm_lectures/parking_function)
