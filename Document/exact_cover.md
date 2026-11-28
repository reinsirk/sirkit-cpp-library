# 完全被覆

候補行を選び、必須列をちょうど1回ずつ、任意列を高々1回覆います。敷き詰めなら「マス」は必須列、「使わなくてもよいピース」は任意列です。候補行はピースを置ける位置ごとに追加します。

| API | 動作 |
|---|---|
| `ExactCover(required, optional = 0)` | 必須列を `[0, required)`、任意列をその直後に用意する。 |
| `add_row(columns)` | 行を追加し、追加順の行番号を返す。$k$ 列で $O(k\log k)$ 時間。 |
| `solve()` | 解の行番号を1組返す。解がなければ `std::nullopt`。 |
| `for_each_solution(visit)` | 解ごとに行番号の `std::span` を渡す。戻り値が `false` なら打ち切る。 |

各行には必須列が1つ以上必要です。列番号の範囲外、重複、空行は `std::invalid_argument` です。列数と管理用の1列の合計が `size_t` を超えると `std::length_error` になります。コールバックに渡す `span` は呼び出し中だけ有効です。

全列数を $C$、行に含まれる列の延べ数を $M$ とすると、探索準備は $O(C+M)$ 時間、使用領域も $O(C+M)$ です。探索は最悪指数時間で、候補が少ない列から試します。

[使用例](../Examples/exact_cover_example.cpp) ／ [Verify](../Verify/current/submissions/exact_cover_abc345_d.cpp)
