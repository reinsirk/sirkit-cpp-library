# 2-SAT

`two_sat`は、bool変数に「条件Aまたは条件B」という制約を加え、全制約を同時に満たす値があるか求めます。[本体](../Graph/Two_SAT.cpp)・[使用例](../Examples/two_sat_example.cpp)を参照してください。

型名・コンストラクタ・`add_clause`・`satisfiable`・`answer`の呼び出しと戻り値は[ACL](https://atcoder.github.io/ac-library/production/document_ja/twosat.html)に合わせています。このライブラリでは型をグローバルに置き、`atcoder::`は付けません。

## API

| API | 動作 |
| --- | --- |
| `two_sat()` / `(int n)` | 空／変数0..n-1を用意する。0<=n<=INT_MAX/2。 |
| `add_clause(i,a,j,b)` | `(x_i==a) || (x_j==b)`を追加する。 |
| `imply(i,a,j,b)` | x_i==aならx_j==b、という制約を追加する。 |
| `force(i,a)` | x_iをaに固定する。 |
| `add_xor(i,j,different=true)` | 2変数を異なる値にする。falseなら同じ値にする。 |
| `satisfiable()` | 全制約を満たせるならtrue。複数回実行でき、制約追加後は再判定する。 |
| `answer()` | 解を`std::vector<bool>`の値で返す。常に長さnで、判定前・UNSAT時の内容は未規定。 |
| `value(i)` | 最後の判定がtrueで、以後制約を追加していない場合の変数iのbool値。 |
| `size()` / `clause_count()` / `reserve_clauses(m)` | 変数数／内部の2条件の節数／m節分を予約。 |
| `reset(n=0)` / `swap(other)` | 制約を捨てn変数へ／O(1)交換。 |

```cpp
two_sat constraints(2);
constraints.add_clause(0,true,1,true); // x0かx1の少なくとも一方がtrue。
constraints.force(0,false);
if (constraints.satisfiable()) {
    auto answer = constraints.answer();
    bool x1 = answer[1]; // true。
}
```

`answer()`は独立したコピーなので、返り値を書き換えても内部の解は変わりません。制約を追加した場合は再判定してから新しい解として使います。`value(i)`だけは最後の判定がtrueであることが前提です。XORは内部では2節として数えます。

N変数・M節に対し、構築はO(1)、制約追加は償却O(1)、`satisfiable()`はO(N+M)時間・追加領域です。`answer()`はO(N)時間・領域、`value(i)`はO(1)です。判定中はbyte配列で値を保持し、`answer()`呼び出し時にbit配列へ変換します。

各変数の真偽を2頂点に変換し、強連結成分分解で矛盾を判定します。再帰を使わず、成分の確定時に同じ変数の真偽が同居していないか確認します。別ファイルのincludeに依存せず、SCCと両方を貼り付けても使えます。コピーは独立、move元は空の未解決状態です。

変数番号と`value()`の解の有効性の検査、検査専用状態は`SKIP_RUNTIME_CHECKS`で除きます。内部のサイズ保護と矛盾の数学的判定は維持します。

[独立参照](../Verify/current/tests/new_scc_two_sat_reference_test.cpp)・[Library Checker](https://judge.yosupo.jp/problem/two_sat)
