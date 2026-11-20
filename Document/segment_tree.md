# Segment Tree

`segtree<S,op,e>`は、点の値を置き換え、区間和・最小値などを求めます。添字は0始まり、区間は左端を含み右端を含まない[l,r)です。公開名・型引数・基本操作はACL形式で、`atcoder::`は付けません。[使用例](../Examples/segment_tree_example.cpp)を参照してください。

Sは値の型、opは左右の区間の答えを結合する関数、e()は空区間の答えです。opは括り方を変えても結果が変わらず、e()と左右どちらの順に結合しても元の値を返す必要があります。左右の順序は保持するため、文字列連結や行列積にも使えます。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
segtree();
explicit segtree(int n);
explicit segtree(const std::vector<S>& values);
segtree(const segtree&) = default;
segtree(segtree&& other);
```

空列、e()をn個並べた列、または指定列から構築します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素にO(N+1)時間・領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## set

```text
void set(int i, S value);
```

指定位置の値を置き換えます。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## transform

```text
template<class F> void transform(int i, F&& f);
```

指定位置の現在値を関数へ渡し、戻り値で置き換えます。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## get

```text
S get(int i) const;
```

指定位置のSを返します。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(1)。

## all_prod

```text
S all_prod() const;
```

列全体を先頭から集約した値を返します。空列は単位元です。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## prod

```text
S prod(int l, int r) const;
```

半開区間[l,r)を左から集約した値を返します。空区間は単位元です。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## max_right

```text
template<bool (*Pred)(S)> int max_right(int l) const;
template<class Pred> int max_right(int l, Pred pred) const;
```

左端lを固定し、述語を満たす最大の右端を返します。

**制約:** 端点は[0,size()]内。pred(単位元)=trueで、区間を伸ばしてfalseの後はtrueへ戻らないこと。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## min_left

```text
template<bool (*Pred)(S)> int min_left(int r) const;
template<class Pred> int min_left(int r, Pred pred) const;
```

右端rを固定し、述語を満たす最小の左端を返します。

**制約:** 端点は[0,size()]内。pred(単位元)=trueで、区間を伸ばしてfalseの後はtrueへ戻らないこと。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## size

```text
int size() const noexcept;
```

要素数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## values

```text
std::vector<S> values() const;
```

現在の列をvectorへコピーして返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(N+1)時間・O(N)追加領域。

## assign

```text
void assign(const std::vector<S>& values);
```

指定した列で再構築します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+新N+1)時間・新列の保持領域。

## reset

```text
void reset(int n);
void reset();
```

nを指定するとe()をn個並べます。引数を省くと、現在の長さを保って全要素をe()に戻します。

**制約:** 共通の前提を満たすこと。

**計算量:** n指定形はO(旧N+新N+1)、引数省略形はO(N+1)。

## clear

```text
void clear() noexcept;
```

確保容量を保って空列にします。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+1)の上界。要素・設定の破棄費用を含みます。

## swap

```text
void swap(segtree& other);
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
std::int64_t op(std::int64_t a,std::int64_t b) { return a+b; }
std::int64_t e() { return 0; }
// 以下はmain内。
segtree<std::int64_t,op,e> tree(std::vector<std::int64_t>{2,4,1});
tree.set(1,9);                // [2,9,1]。
auto sum = tree.prod(0,2);    // 11。
```

`prod`はopで指定した演算で区間を集約します。上の例では区間和を返します。同じ呼出形式の関数オブジェクトも型引数に指定できます。

## 境界探索と利用上の注意

max_rightは左端lを固定して右へ、min_leftは右端rを固定して左へ区間を伸ばします。例えば[1,2,3,4]で「合計5以下」を条件にすると、max_right(1,pred)は3です。[1,3)の合計は5、[1,4)は9になります。関数bool pred(S)を定義し、`max_right<pred>(l)`・`min_left<pred>(r)`とも書けます。

述語は同じ値に同じ結果を返し、pred(e())がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。区間和の上限判定なら全要素が非負のときに使えます。単調性を確認する全要素走査は行いません。

演算・値のコピーをO(1)として、領域はO(N+1)です。assign・reset(n)は旧列の破棄を含めO(旧N+新N+1)、reset()はO(N+1)。列長・添字・区間端はintで、0<=p<N・0<=l<=r<=Nが前提です。値の演算は途中計算も型の範囲内に収めます。

不正な添字・区間・述語の単位元条件は既定で例外により検査し、`SKIP_RUNTIME_CHECKS`で除きます。容量・添字の保護は維持します。コピーは独立、移動元は空列として再利用できます。演算等の例外時に内容の強い保証は行いません。

公開APIは[ACLのsegtree](https://atcoder.github.io/ac-library/production/document_ja/segtree.html)、数学的参考は[モノイド設計](https://info.atcoder.jp/entry/algorithm_lectures/range_query_monoid_design)です。実装は本ライブラリ用のCC0です。

## operator=

```text
segtree& operator=(const segtree& other);
segtree& operator=(segtree&& other);
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
