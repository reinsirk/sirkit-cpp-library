# 区間加算後の数列のID

すべて0の数列に区間加算を行い、現在の数列を整数のIDで表します。同じオブジェクトでIDが一致するのは、全要素が一致するときです。更新の順序や回数にかかわらず、元の数列へ戻ると同じIDになります。

| API | 内容 |
| --- | --- |
| `RangeAddStateIds(n)` | `n` 要素を0で初期化する。$O(1)$。 |
| `size()` / `id()` | 要素数と現在のID。初期IDは `0`。各 $O(1)$。 |
| `add(l, r, delta)` | 半開区間 `[l,r)` に加算してIDを返す。期待償却 $O(\log(n+1))$ 時間、追加 $O(\log(n+1))$ 領域。 |

値は法 $2^{64}$ で管理します。IDは `std::uint32_t` です。過去の数列と照合するために、各更新で現れた差分の節点を保持します。

```cpp
sirkit::RangeAddStateIds data(100);
const auto first = data.add(10, 20, 3);
data.add(10, 20, -3); // IDは0
assert(data.add(10, 20, 3) == first);
```

[使用例](../Examples/range_add_state_ids_example.cpp) ／ [Verify](../Verify/current/submissions/range_add_state_ids.cpp)
