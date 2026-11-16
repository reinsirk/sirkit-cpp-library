# Lazy Segment Tree

`lazy_segtree<S,op,e,F,mapping,composition,id>`は、配列の区間をまとめて更新し、区間和・最小値などを求めます。添字は0始まり、区間は左端を含み右端を含まない`[l,r)`です。[加算・区間和の使用例](../Examples/lazy_segment_tree_add_sum_example.cpp)、[代入も扱う使用例](../Examples/lazy_segment_tree_example.cpp)を参照してください。

演算とS・FのコピーをO(1)として、構築はO(N+1)時間・領域、更新と区間照会はO(log(N+1))です。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## 型と関数の設定

- `S`: 区間の答えを表す型。 例: 合計と要素数の組。
- `op(left,right)`: 左右の区間の答えを結合する。 例: 合計と要素数をそれぞれ足す。
- `e()`: 空区間の答え。opに混ぜても結果を変えない。 例: 合計0・要素数0。
- `F`: 1回の更新を表す型。 例: 各要素に足す量。
- `mapping(f,x)`: 更新fを区間の答えxへ反映する。 例: 合計に「加算量×要素数」を足す。
- `composition(newer,older)`: olderの後にnewerを適用する更新を作る。 例: 加算量を足す。
- `id()`: 何も変更しない更新。 例: 加算量0。

opは括り方で結果が変わらないことが前提です。左右の順序は保持します。mappingは「結合してから更新」と「更新してから結合」で同じ結果になるよう定義します。**e()は空区間、id()は何もしない更新**です。

## コンストラクタ

```text
lazy_segtree();
explicit lazy_segtree(int n);
explicit lazy_segtree(const std::vector<S>& values);
lazy_segtree(const lazy_segtree&) = default;
lazy_segtree(lazy_segtree&& other);
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

## get

```text
S get(int i);
```

指定位置のSを返します。遅延伝播のため非constです。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## apply

```text
void apply(int i, F f);
void apply(int l, int r, F f);
```

1要素または半開区間[l,r)へ更新fを適用します。空区間には何もしません。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## prod

```text
S prod(int l, int r);
```

半開区間[l,r)を左から集約した値を返します。空区間は単位元です。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## all_prod

```text
S all_prod();
```

列全体を先頭から集約した値を返します。空列は単位元です。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## max_right

```text
template<bool (*pred)(S)> int max_right(int l);
template<class Pred> int max_right(int l, Pred pred);
```

左端lを固定し、述語を満たす最大の右端を返します。

**制約:** 端点は[0,size()]内。pred(単位元)=trueで、区間を伸ばしてfalseの後はtrueへ戻らないこと。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## min_left

```text
template<bool (*pred)(S)> int min_left(int r);
template<class Pred> int min_left(int r, Pred pred);
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
std::vector<S> values();
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

引数付きはe()をn個並べ、省略形は現在の長さで全要素をe()へ戻します。

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
void swap(lazy_segtree& other);
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

`prod`はopで決めた集約で、掛け算とは限りません。applyは設定した更新、setは直接の置換です。assign・reset(n)はO(旧N+新N+1)、reset()はO(N+1)です。

## 区間加算・区間和の例

```cpp
struct Sum { std::int64_t value; int length; };
using F = std::int64_t;
Sum op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(F add, Sum x) { return {x.value + add * x.length, x.length}; }
F composition(F newer, F older) { return newer + older; }
F id() { return 0; }
using Tree = lazy_segtree<Sum, op, e, F, mapping, composition, id>;
// 以下はmain内。
Tree tree(std::vector<Sum>{{1,1}, {2,1}, {3,1}});
tree.apply(0, 2, 3);                 // 列は[4,5,3]。
auto sum = tree.prod(0, 3).value;     // 12。
tree.set(1, Sum{10,1});              // 列は[4,10,3]。
```

lengthは、区間への加算量を「add×要素数」で求めるために持ちます。**1要素は{値,1}、空区間は{0,0}**です。Tree(n)やresetではlengthも0になるため、値0の配列は`Tree tree(std::vector<Sum>(n,Sum{0,1}))`で作ります。

代入・乗算を含む更新は[アフィン変換の例](../Examples/lazy_segment_tree_example.cpp)を参照してください。

## 制約

境界探索には`max_right<pred>(l)`・`min_left<pred>(r)`の関数テンプレート形式もあります。述語は同じ値に同じ結果を返し、`pred(e())`がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。

本ライブラリ独自の拡張として`mapping(F,S,std::size_t length)`も使えます。区間長を受け取れるため、上の例のlengthをSから省けます。2引数形も呼べる場合はそちらを優先します。更新によって区間長を変えてはいけません。

列長・添字・区間端はintで、`0<=p<N`・`0<=l<=r<=N`が前提です。NはINT_MAX以下かつ内部容量に収めます。利用側の演算は途中計算も含めて型の範囲内に収め、Mintが必要なら利用側で用意します。

不正な添字・区間・述語の単位元条件は既定で例外により検査します。`SKIP_RUNTIME_CHECKS`で入力検査を除き、容量・内部添字の保護は維持します。照会も遅延伝播で内部状態を変更するため、get・prod・境界探索・valuesは非constです。コピーは独立、ムーブ元は空列として再利用できます。演算やコピーが例外を投げる場合、内容の強い保証は行いません。

型引数と基本操作は[ACL](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html)に合わせています。


## operator=

```text
lazy_segtree& operator=(const lazy_segtree& other);
lazy_segtree& operator=(lazy_segtree&& other);
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
