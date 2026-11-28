# 円内・円周の格子点数

円の内部と円周にある整数格子点を数えます。小数の座標も一律の倍率で整数に直せば、境界を正確に判定できます。

`sirkit::geo::count_disk_lattice_points(x, y, radius, unit = 1)` は、中心 $(x/\text{unit}, y/\text{unit})$、半径 $\text{radius}/\text{unit}$ の円を扱います。戻り値の `closed` は円周を含む個数、`boundary` は円周だけの個数です。

`unit` は正、`radius` は $0$ 以上 $2^{31}-1$ 以下です。範囲外では `std::invalid_argument` を送出します。$O(1+\text{radius}/\text{unit})$ 時間、$O(1)$ 領域です。

`count_disk_lattice_points(1, 0, 2, 2)` は中心 $(0.5,0)$、半径 $1$ の円で、`closed == 2`、`boundary == 0` になります。

[使用例](../Examples/disk_lattice_points_example.cpp) ／ [Verify](../Verify/current/submissions/disk_lattice_points_abc191_d.cpp)
