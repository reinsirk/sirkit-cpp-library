# Wavelet Matrix

`WaveletMatrix<T>`は、変更しない整数列に対して「区間内でk番目に小さい値」「指定値の個数」などを照会します。区間は0始まりの半開区間[l,r)、順位kも0始まりです。[使用例](../Examples/wavelet_matrix_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `WaveletMatrix(values)` / `build(values)` | vectorまたはspanの列で構築 / 再構築する。既定構築は空列。 |
| `access(i)` / `operator[](i)` | 添字iの値を返す。 |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 区間内を昇順 / 降順に並べたk番目を返す。0<=k<r-l。 |
| `range_min(l,r)` / `range_max(l,r)` | 非空区間の最小値 / 最大値を返す。 |
| `count(l,r,x)` / `rank(x,l,r)` | 区間内のxの個数を返す。 |
| `count_less(l,r,x)` / `count_less_equal(l,r,x)` | x未満 / x以下の個数を返す。 |
| `range_freq(l,r,lower,upper)` | lower以上upper未満の個数。lower>=upperなら0。 |
| `predecessor(l,r,x)` / `successor(l,r,x)` | x未満の最大値 / x以上の最小値をoptional<T>で返す。なければnullopt。 |
| `size()` / `empty()` / `clear()` / `swap(other)` | 要素数 / 空か / 空にする / 交換する。 |

```cpp
WaveletMatrix<std::int64_t> matrix(std::vector<std::int64_t>{-5,7,2,-5,10});
auto value = matrix.kth_smallest(0,5,1); // -5。小さい順で2番目。
auto count = matrix.count(0,5,-5);      // 2。
auto next = matrix.successor(0,5,3);    // optionalに7が入る。
```

## 前提と計算量

Tはbool以外の64ビット以下の整数型です。符号付き型の最小値・最大値も扱えます。長さはUINT32_MAX以下で、構築時に確認します。

値の順序を保つ符号へ変換して最小値を引き、必要なビット幅B（0〜64）だけを扱います。構築はO(N(1+B))、照会はO(1+B)、格納はO(1+B ceil(N/64))語、構築中の追加領域はO(N)です。全値が同じならB=0となり、照会はO(1)です。

コピーは独立、buildとコピー代入は失敗時に元の列を維持します。ムーブ元は空列です。`SKIP_RUNTIME_CHECKS`で添字・区間・順位の検査を除きますが、長さの表現上限は維持します。

対応CPUではPOPCNT・AVX2を使います。`SIRKIT_DISABLE_SIMD`で追加の専用実装とCPU判定を除き、標準C++経路へ切り替えます。コンパイラ自身の自動ベクトル化は通常の設定に従います。経路の条件と測定は[Audit](../Audit/)を参照してください。

数学的参考は[AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/wavelet_matrix_basic)です。実装は本ライブラリ用に独立に組み立てています。
