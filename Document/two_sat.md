# 2-SAT

`two_sat`は、bool変数に「条件Aまたは条件B」という制約を加え、全制約を同時に満たす値があるか求めます。[本体](../Graph/Two_SAT.cpp)・[使用例](../Examples/two_sat_example.cpp)を参照してください。

型名・コンストラクタ・`add_clause`・`satisfiable`・`answer`の呼び出しと戻り値は[ACL](https://atcoder.github.io/ac-library/production/document_ja/twosat.html)に合わせています。型は`sirkit::two_sat`です。

## API

### two_sat

```text
two_sat();
explicit two_sat(int n);
two_sat(const two_sat&);
two_sat(two_sat&& other) noexcept;
```

空、またはn個のbool変数を持つ制約なしのモデルを作ります。

**制約:** 0<=n<=INT_MAX/2。

**計算量:** O(1)。コピーは保持量に比例、ムーブはO(1)。

### add_clause

```text
void add_clause(int i, bool a, int j, bool b);
```

条件(x_i==a)または(x_j==b)を追加します。

**制約:** i,jは0..N-1。

**計算量:** 償却O(1)。再確保時は現在の保持節数に比例します。

### imply

```text
void imply(int i, bool a, int j, bool b);
```

x_i==aならx_j==bという含意を追加します。

**制約:** i,jは0..N-1。

**計算量:** 償却O(1)。

### force

```text
void force(int i, bool a);
```

変数x_iをaに固定する制約を追加します。

**制約:** 0<=i<N。

**計算量:** 償却O(1)。

### add_xor

```text
void add_xor(int i, int j, bool different = true);
```

different=trueなら2変数を異なる値、falseなら同じ値にします。内部では2節を追加します。

**制約:** i,jは0..N-1。

**計算量:** 償却O(1)。

### satisfiable

```text
bool satisfiable();
```

全制約を満たせるならtrue。複数回実行でき、制約追加後は再判定する。

**制約:** 構築済みの入力が共通の前提を満たすこと。

**計算量:** O(N+M)時間・追加領域。

### answer

```text
std::vector<bool> answer();
```

解を`std::vector<bool>`の値で返す。常に長さnで、判定前・UNSAT時の内容は未規定。

**制約:** 常に長さNですが、判定前・UNSAT時の内容は未規定です。

**計算量:** O(N)時間・領域。

### value

```text
bool value(int i) const;
```

最後の充足可能な解での変数のbool値を返します。

**制約:** 最後のsatisfiable()がtrueで、以後制約を追加していないこと。0<=i<size()。

**計算量:** O(1)。

### size

```text
int size() const noexcept;
```

変数数を返します。

**制約:** なし。

**計算量:** O(1)。

### clause_count

```text
std::size_t clause_count() const noexcept;
```

内部の2条件の節の個数を返します。

**制約:** なし。

**計算量:** O(1)。

### reserve_clauses

```text
void reserve_clauses(std::size_t count);
```

指定節数分の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reset

```text
void reset(int n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 0<=n<=INT_MAX/2。

**計算量:** 旧領域の破棄費用。

### swap

```text
void swap(two_sat& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


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

N変数・M節に対し、構築はO(1)、制約追加は償却O(1)、`satisfiable()`はO(N+M)時間・追加領域です。`answer()`はO(N)時間・領域、`value(i)`はO(1)です。

別ファイルのincludeは不要で、SCCとの併用も可能です。コピーは独立、move元は空の未解決状態です。

変数番号と`value()`の解の有効性の検査、検査専用状態は`SKIP_RUNTIME_CHECKS`で除きます。内部のサイズ保護と矛盾の数学的判定は維持します。

[独立参照](../Verify/current/tests/new_scc_two_sat_reference_test.cpp)・[Library Checker](https://judge.yosupo.jp/problem/two_sat)

## 構築・状態と補助型

### operator=

```text
two_sat& operator=(two_sat other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
