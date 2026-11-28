# 編集距離と操作列の復元

`edit_alignment(source, target)` は元の列を先の列にそろえる最小編集回数と、その操作列を返します。文字列は `std::string_view`、文字以外の列は `std::span{values}` で渡します。[本体](../String/Edit_Alignment.cpp)

結果の `distance` は挿入・削除・置換の合計回数です。`steps` を先頭から走査すると、どの要素を対応づけるかが分かります。両列の添字を $0$ から始め、操作ごとに進めます。

| 操作 | 元の列 | 先の列 |
| --- | --- | --- |
| `EditAction::kMatch`（一致） | 進める | 進める |
| `EditAction::kReplace`（置換） | 進める | 進める |
| `EditAction::kErase`（削除） | 進める | 保持 |
| `EditAction::kInsert`（挿入） | 保持 | 進める |

`std::span` の場合は、第3引数に一致判定を指定できます。判定には元の値、先の値の順で渡ります。長さを $n,m$ とすると、$O(nm+n+m+1)$ 時間です。操作列を除く追加領域は $O(1+\min(n,m)+\log(\max(n,m)+1))$ です。

```cpp
const auto alignment = sirkit::edit_alignment("kitten", "sitting");
assert(alignment.distance == 3);
```

[操作列をたどる使用例](../Examples/edit_alignment_example.cpp)・[AtCoder Verify](../Verify/current/submissions/edit_alignment_sequence_matching.cpp)
