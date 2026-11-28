# ビット集合で持つ非決定性有限オートマトン

`BitmaskNfa<A> nfa(n)` で、文字を `0` から `A-1`、状態を `0` から `n-1` とするオートマトンを作ります。文字種数と状態数は1以上64以下です。初期状態は既定で0です。

- `add_transition(from, symbol, to)` で文字付き遷移、`add_epsilon(from, to)` で文字を読まずに進む遷移を追加します。
- `set_accepting(state)` で受理状態を指定します。第2引数に `false` を渡すと解除できます。
- `determinize(max_states)` で到達可能な状態を集めてDFAにします。DFAの状態数が指定した上限を超えると `length_error` です。上限は省略できます。
- DFAの `accepts(word)` は単語を受理するか判定します。`count_matches<T>(choices)` は各位置の文字候補から受理される単語の個数を数えます。候補は文字番号のビット集合で渡します。
- 自分でDPを書く場合は、DFAの初期状態を0とし、`transition(state, symbol)` で遷移先、`accepting(state)` で受理条件を取得できます。

たとえば `A=2` なら候補 `{3, 1, 2}` が表す単語は `001` と `101` です。DFAは単語ごとに遷移が一意なので、NFAで同じ単語に複数の経路があっても1回だけ数えます。個数が大きい場合は剰余整数を `T` に指定できます。

DFAの状態数 $D$ は最大 $2^n$ です。変換には平均 $O(nA(n+D))$ 時間と $O(A(n+D))$ 領域を使います。長さ $L$ の入力に対して受理判定は $O(L)$ 時間、数え上げは $O(D(1+LA))$ 時間・追加領域 $O(D)$ です。

[使用例](../Examples/bitmask_nfa_example.cpp) ／ [Verify](../Verify/current/submissions/bitmask_nfa_wildcard.cpp)
