# 決定性有限オートマトンの状態数を最小化

`minimize_dfa(alphabet, transitions, accepting, start)` は、同じ言語を受理する最小のDFAを返します。状態と文字の番号は0始まりです。元の遷移は `transitions[state * alphabet + symbol]`、受理状態は `accepting[state] != 0` で渡します。初期状態 `start` は省略すると0になります。

- 戻り値の初期状態は0です。`transition(state, symbol)`、`accepting(state)` で遷移先と受理フラグを取得できます。
- `state_map()[old]` は元の状態に対応する番号です。元の初期状態から到達不能なら `DfaMinimization::kUnreachable` になります。

Hopcroft法で、同じ続きを受理できる状態を一緒にします。状態数 $N$、文字種数 $A$ で $O(N(1+A)(1+\log N))$ 時間、$O(N(1+A))$ 領域を使います。入力のDFAには全状態・全文字に対する遷移を与えてください。

[使用例](../Examples/dfa_minimization_example.cpp) ／ [Verify](../Verify/current/submissions/dfa_minimization_wildcard.cpp)
