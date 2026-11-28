# 石取りゲームの Grundy 数

決められた個数の石を山から取り除き、動けなくなった側が負けるゲームを扱います。複数の山では、各手番に山を1つ選んで石を取ります。

| API | 動作 |
|---|---|
| `SubtractionGame(moves, limit = 0)` | 取れる石の個数と、計算する山の上限を指定する。各個数は正。 |
| `extend(limit)` | 山の上限を延ばす。作成済みの Grundy 数は再利用する。 |
| `grundy(stones)` | 山1つの Grundy 数を $O(1)$ で返す。 |
| `winning_move(piles)` | 必勝手の `{pile, removed}` を返す。山番号は0始まり。先手が勝てなければ `std::nullopt`。 |

`moves` の長さが $K$、上限が $L$ なら、前計算は $O(K\log(K+1)+(K+1)L)$ 時間、$O(K+L+1)$ 領域です。山が $P$ 個の必勝手の取得は $O(P+K)$ 時間。計算済みの上限より大きい山を照会すると `std::out_of_range` になります。

[使用例](../Examples/subtraction_game_example.cpp) ／ [Verify（山1つ）](../Verify/current/submissions/subtraction_game_dp_k.cpp) ／ [Verify（複数の山）](../Verify/current/submissions/subtraction_game_tessoku_a34.cpp)
