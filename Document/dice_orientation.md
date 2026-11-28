# サイコロの向き

`DiceOrientation` は6面のサイコロの姿勢を扱います。ラベルの配列は上・下・北・南・東・西の順です。

- `rolled(DiceRoll::North)` は北に転がした後の姿勢を返します。転がす前の上面が北面になります。
- `original_face(DiceFace::Top)` は、今上を向いている面が初期状態ではどちらを向いていたかを返します。`apply(labels)` で今の各面のラベルを取得できます。
- `id()` は0から23までの番号を返します。`all()` は番号順の全姿勢です。`from_faces(top,north)` は上と北に来る元の面から姿勢を作り、その2面が隣り合わなければ `nullopt` を返します。

`canonical_dice_faces(labels)` は、24通りのラベル列のうち辞書順で最小の列を返します。同じ値なら回転で一致するサイコロです。面ラベルは重複していてもかまいません。

[使用例](../Examples/dice_orientation_example.cpp) ／ [転がすVerify](../Verify/current/submissions/dice_orientation_yuki1572.cpp) ／ [一致判定のVerify](../Verify/current/submissions/dice_orientation_aoj_itp1_11_c.cpp)
