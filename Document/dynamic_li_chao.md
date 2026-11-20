# 動的Li Chao Tree

整数座標の閉区間で、追加した直線・線分の最小値を取得します。座標圧縮や照会座標の事前登録は不要です。

## APIと前提

`DynamicLiChaoTree<T, kInf>(xmin, xmax, inf=kInf)`で定義域`[xmin,xmax]`と正の番兵infを指定します。Tは符号付き64bit以下の整数で、既定はint64_t。kInfの既定は2^60で、Tで表せない場合はTの最大値です。テンプレート引数による指定も維持します。

### DynamicLiChaoTree

```text
DynamicLiChaoTree(T xmin, T xmax, T inf = kInf);
```

閉区間[xmin,xmax]で使う空の木を構築します。

**制約:** xmin<=xmax、inf>0。

**計算量:** O(1)。

### add_line

```text
void add_line(T a, T b);
```

定義域全体へ直線y=a*x+bを追加します。

**制約:** 対象区間で数学的なa*x+bがTに収まり、設定したinf未満。

**計算量:** O(log W)。

### update

```text
void update(T a, T b);
```

定義域全体へ直線y=a*x+bを追加します。

**制約:** 対象区間で数学的なa*x+bがTに収まり、設定したinf未満。

**計算量:** O(log W)。

### add_segment

```text
void add_segment(T a, T b, T left, T right);
```

閉区間[left,right]へ線分を追加します。left>rightは無操作です。

**制約:** 空区間でないとき両端は定義域内で、対象区間の値がT内かつinf未満。

**計算量:** O(log² W)。

### update_line_segment

```text
void update_line_segment(T a, T b, T left, T right);
```

閉区間[left,right]へ線分を追加します。left>rightは無操作です。

**制約:** 空区間でないとき両端は定義域内で、対象区間の値がT内かつinf未満。

**計算量:** O(log² W)。

### query

```text
T query(T x) const;
```

最小値を返し、被覆する線がなければ設定したinfです。

**制約:** xは定義域内。

**計算量:** O(log W)。

### query_line

```text
std::optional<Line> query_line(T x) const;
```

最小値を与えるLineを返します。同じ最小値を与える線が複数あれば任意の1本を返し、対象区間がxを含む直線や線分がなければnulloptを返します。

**制約:** xは定義域内。

**計算量:** O(log W)。

### infinity

```text
T infinity() const noexcept;
```

設定したinfを返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

追加した直線・空でない線分がなければtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### domain_min

```text
T domain_min() const noexcept;
```

定義域の左端を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### domain_max

```text
T domain_max() const noexcept;
```

定義域の右端を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### stored_nodes

```text
std::size_t stored_nodes() const noexcept;
```

格納しているノードの数を返します。照会でノード数は増えません。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

全ての線を消し、容量を再利用します。

**制約:** 追加の制約はありません。

**計算量:** 格納ノード数MにO(M)。

### reserve

```text
void reserve(std::size_t n);
```

nノード分の容量を予約します。直線本数ではありません。

**制約:** ノード容量の上限以下。

**計算量:** 再確保時O(M)、予約済みならO(1)。

### Line::Line

```text
Line();
Line(T a, T b);
```

既定ではy=kInf、指定時はy=a*x+bの係数を持ちます。kInfは木ごとのinfとは別です。

**制約:** 係数はTの範囲内。

**計算量:** O(1)。

### Line::get

```text
T get(T x) const noexcept;
```

直線の値a*x+bを返します。

**制約:** 数学的な最終値がTで表現可能。

**計算量:** O(1)。

### Line::over

```text
bool over(const Line& other, T x) const noexcept;
```

xでotherより小さい値を与えればtrueです。

**制約:** 両直線の数学的な最終値がTで表現可能。

**計算量:** O(1)。


各直線・線分の**対象区間内**で、数学的な`a*x+b`が`T`に収まり、指定した`inf`未満である必要があります。中間の積だけが範囲を超え、切片との相殺で最終値が収まる場合も扱えます。線分の対象区間外の値は前提に含めません。

既定では座標と両端の値を検査します。線形関数なので、両端の確認で対象区間内の値域を判定できます。`SKIP_RUNTIME_CHECKS`で入力検査を除いても、内部のノード数上限は維持します。

## 方式と計算量

座標数を`W`、格納ノード数を`M`とすると、直線追加・照会は`O(log W)`、線分追加は`O(log² W)`、使用メモリは`O(M)`です。全域の64ビット整数座標でも高さは最大64です。

空の構築・状態取得はO(1)、clearはO(M)、reserveは再確保時O(M)です。コピーはO(M)で独立した木を作り、moveはO(1)です。コピー・move・clear後もinfの設定を保持します。コピー代入ではノードのコピーを先に完了させ、確保失敗時に元の定義域とノードを維持します。

[完全な使用例](../Examples/dynamic_li_chao_example.cpp)
