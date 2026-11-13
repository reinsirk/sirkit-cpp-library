# 多項式の多点評価・補間・Taylor shift

[ProductTreeと多項式操作](../Math/Polynomial.cpp)は、法998244353で多数の点の評価・値からの補間・f(X+c)への変換を行います。Mintは不要です。同梱の[FormalPowerSeries](formal_power_series.md)・[NTT](ntt.md)とは共通ガードを使い、同時に貼り付けられます。[使用例](../Examples/polynomial_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `ProductTree(points)` / `assign(points)` | 各点の(X−x_i)をまとめた積木を構築する。反復評価では同じ木を再利用できる。 |
| `size()` / `empty()` / `points()` | 点数 / 空か / 正規化済み点列のconst参照。 |
| `polynomial()` | 全点の積Π(X−x_i)をFormalPowerSeriesで返す。空点列では定数1。 |
| `evaluate(poly)` | 各点での値をvector<uint32_t>で返す。点の重複は可、結果は入力点の順。 |
| `interpolate(values)` | 点と値から次数n未満のFormalPowerSeriesを返す。点は法で相異なり、値は点数個。 |
| `MultipointEvaluate(poly,points)` / `PolynomialInterpolate(points,values)` | 一時積木を使う簡易版。 |
| `TaylorShift(poly,c)` | f(X+c)のFormalPowerSeriesを、入力と同じ係数数で返す。 |
| `swap(other)` | 点列と積木をO(1)で交換する。 |

```cpp
const FormalPowerSeries f{1,2,3}; // 1+2X+3X²。
ProductTree tree(std::vector<int>{0,1,2});
auto values = tree.evaluate(f); // {1,6,17}。
auto restored = tree.interpolate(values); // fと一致。
auto shifted = TaylorShift(f,1); // {6,8,3}。
```

polyはFormalPowerSeriesまたは整数係数vectorです。点・係数・値は符号付き／符号なし64ビット以下の整数を法へ正規化し、INT64_MIN・UINT64_MAXも扱えます。補間結果はn係数を保持し、末尾0も残します。空の点列からの評価・補間は空です。

## 前提と計算量

点数nは2^22−1以下、係数数dは2^22以下です。補間の重複点は数式上の分母0となるため、`SKIP_RUNTIME_CHECKS`でもdomain_errorです。値列の長さ検査は省略できますが、容量・数学的な0判定は維持します。

M(k)=O(k log(k+1))とします。積木の構築はO(1+M(n)log(n+1))時間・O(n log(n+1))保持。評価はO(1+M(max(n,d))log(n+2)+n+d)時間・O(n+d)追加領域です。補間はO(1+M(n)log(n+1))時間・O(n log(n+1))追加領域、TaylorShiftはO(1+M(d))時間・O(d)追加領域です。

評価は子の積による剰余を繰り返して葉の値を求めます。補間は積の微分から分母を作って各点の寄与を結合し、TaylorShiftは階乗を使う畳み込みで求めます。内部手順を利用側で呼び分ける必要はありません。

ProductTreeは点と積を独立に所有します。コピーは独立、別個体へのmove元は空で再利用可です。assignとコピー代入は失敗時に元を保ち、評価・補間は木を変更しません。同梱FPSの根cacheは同一スレッドで共有し、必要なら`FormalPowerSeries::clear_cache()`で解放します。追加SIMD・CPU判定は`SIRKIT_DISABLE_SIMD`で除外できます。

数式・用途の参考は[Nyaanの多点評価](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html)、Verifyは[Polynomial Interpolation](https://judge.yosupo.jp/problem/polynomial_interpolation)などです。積木・評価・補間の実装は独立に構成し、NTT/FPSは既存の自作CC0部品を同期しています。
