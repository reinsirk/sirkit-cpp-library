# 行列の回転と反転

`Algorithm/rotate.cpp`は矩形の`vector<vector<T>>`と、文字盤面の`vector<string>`を回転・反転します。コピー版は入力を変更せず、独立した行列を返します。[使用例](../Examples/matrix_transform_example.cpp)を参照してください。

## API

| 操作 | 結果 |
| --- | --- |
| `rotate_90r(a)` / `rotate_90l(a)` | 時計回り / 反時計回りに90度回転したコピー。 |
| `rotate_180(a)` | 180度回転したコピー。 |
| `rotate_quarter(a,turns)` | 時計回りの回数を4で正規化して回転。負数・INT_MINも可。 |
| `transpose(a)` | 行と列を入れ替えたコピー。 |
| `flip_i(a)` / `flip_j(a)` | 上下 / 左右反転したコピー。 |
| `flip_i_inplace(a)` / `flip_j_inplace(a)` | 行列自体を上下 / 左右反転。戻り値なし。 |

```cpp
std::vector<std::string> board{"abc", "def"};
auto rotated = rotate_90r(board); // {"da","eb","fc"}。
flip_j_inplace(board);           // boardは{"cba","fed"}。
// rotatedはboardの変更に影響されない。
```

## 計算量と前提

H行W列として、要素のコピー・交換をO(1)とすると、コピー版はO(H+HW)時間・返却値にO(H+HW)領域です。上下の直接反転は行の交換だけでO(H)時間・追加O(1)、左右の直接反転はO(H+HW)時間・追加O(1)です。

コピー版のTはコピー構築可能であればよく、既定構築・代入できない型も使えます。左右の直接反転では要素の交換が可能なことが前提です。コピー版で例外が出ても入力は不変で、直接反転は例外までの変更が残る場合があります。

入力は矩形が前提です。通常処理で参照する行の幅を確認し、`SKIP_RUNTIME_CHECKS`で検査を省きます。幅0の回転・転置では先頭以外の行、上下の直接反転では奇数行数の中央行を見ないため、その部分の検査は保証しません。

## 空行列とSIMD

外側が空の場合、互換性のためrotate_90r・flip_i・flip_jは1本の空行を返し、他のコピー版は空を返します。H本の空行は90度回転・転置では空になり、反転・180度回転ではH本の空行を維持します。

GCC／Clangの対応CPUでは、一部の大きな行列の回転・転置にAVX2を使います。`SIRKIT_DISABLE_SIMD`をinclude前に定義すると専用処理・ヘッダー・CPU判定を除き、標準C++の処理を使います。型・寸法による切り替えやタイル処理の詳細は[Audit](../Audit/)、正しさは[独立参照テスト](../Verify/current/tests/matrix_reference_test.cpp)を参照してください。
