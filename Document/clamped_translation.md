# 加算と上下限の合成

加算、下限の適用、上限の適用を並べた関数を、合計の加算量と上下限で保持します。同じ操作列を多数の値に適用するときに使います。

## `ClampedTranslation()`

何も変更しない恒等関数を作ります。

## `then_add(delta)`

現在の出力に `delta` を加える操作を追加します。

## `then_chmax(low)`

現在の出力が `low` 未満なら、`low` に置き換える操作を追加します。

## `then_chmin(high)`

現在の出力が `high` を超えていれば、`high` に置き換える操作を追加します。

## `then(after)`

現在の関数を適用し、その結果に `after` を適用する新しい関数を返します。元の状態は更新しません。

## `apply(x)`

合成した関数に `x` を入れた結果を返します。

各操作は $O(1)$ 時間・領域です。有限の上下限、加算量の合計、返り値は `std::int64_t` に収めてください。上下限の適用で返り値が収まる場合は、途中の加算が型の範囲を超えても構いません。

## 使用例

```cpp
sirkit::ClampedTranslation filter;
filter.then_add(3).then_chmax(0).then_chmin(10);
assert(filter.apply(-6) == 0);
assert(filter.apply(1) == 4);
assert(filter.apply(20) == 10);
```

[実装](../Algorithm/Clamped_Translation.cpp)・[使用例の全体](../Examples/clamped_translation_example.cpp)・[Verify](../Verify/current/submissions/clamped_translation.cpp)
