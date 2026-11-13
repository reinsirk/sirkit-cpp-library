# XOR基底

`Math/Xor_Basis.cpp` の `XorBasis` は、64ビット符号なし整数をGF(2)上のベクトルとみなし、部分集合のXORで生成される線形空間を保持します。固定配列だけを使い、追加の動的確保はありません。

```cpp
XorBasis basis;
basis.insert(3);
basis.insert(5);
std::cout << basis.max_xor() << '\n'; // 6
std::cout << *basis.kth(2) << '\n';   // 5（0,3,5,6の順）
std::cout << basis.contains(6) << '\n'; // 1
```

最高位の1が異なるベクトルを1本ずつ持ちます。`insert(x)` は上のビットから消去し、新しい最高位を作れれば基底へ追加してtrue、既存の線形結合ならfalseを返します。`rank()` は独立なベクトルの数で、0から64です。`contains` も同じ消去で判定します。

空部分集合のXORは0です。そのため、入力がなくても `contains(0)` はtrueで、`kth(0)` は0を返します。非空の部分集合が必要なら `contains_nonempty` を使います。0の挿入や、従属する入力の挿入が一度でもあると `has_nonempty_zero()` がtrueになります。同じ値を2回追加した場合も、2つの異なる入力位置を使って非空XOR=0を作れます。

`merge(other)` は両方に挿入した入力を合わせた意味です。相手の基底と従属性を取り込みます。共通の入力値は重複した位置として扱い、自己結合でも元の各入力を2回ずつ含む意味になります。非空の入力を自己結合した後は非空XOR=0が作れます。

`min_xor(seed)` と `max_xor(seed)` は、seedに空部分集合を含む任意の部分集合XORを加えた最小値・最大値です。これは非空条件を付けた最適値ではありません。`kth(k)` は、作れる相異なるXOR値を符号なし整数の昇順に並べた0始まりk番目です。内部で下位のピボットを消去してから順位の各ビットを対応させます。範囲外は `std::nullopt` です。rankが64のときも64ビットシフトによる未定義動作を避け、全ての `std::uint64_t` の順位を受け付けます。

`insert`、`contains`、`contains_nonempty`、`min_xor`、`max_xor` は O(64)、`merge` と `kth` は O(64²) です。`rank` と `has_nonempty_zero` は O(1)、`clear` と `swap` は O(64) です。コピー・移動は固定配列の値コピーなので、移動元にも内容が残ります。削除や、表現に使った入力位置の復元は行いません。

数学的構成の参考は [Luzhiledのbinary-basis](https://ei1333.github.io/library/math/matrix/binary-basis.hpp) です。64本のピボット配列、非空0の状態、rank64の順位取得として独立に実装しています。
