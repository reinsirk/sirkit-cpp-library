# HashSet

`DataStructure/Hash_set.cpp`は、キーの値を所有する開番地法の集合です。`string_view`などの参照先は所有せず、登録中は参照先が有効で、キーとして不変である必要があります。順序は持ちません。`GenericHash`は整数・文字列・pair・tuple・列などに対応し、独自の同値関係には対応するHasherとKeyEqualを渡します。

同じ要素を持つ`HashSet`は、内部のハッシュ種・列挙順・再配置の違いによらず、外側の`HashMap`や`HashSet`のキーとして同じハッシュ値になります。要素に独自の同値関係を使う集合をキーにするときは、その同値関係に合う外側のHasherも指定してください。

## 基本操作

`insert(key)`は既存または追加した要素のiteratorを返します。`insert_checked(key)`と`emplace(args...)`は`{iterator, 追加の有無}`を返します。`contains(key)`と`count(key)`は追加を行いません。検索・追加は期待償却O(1)で、列キーのハッシュ計算には列長に比例する費用がかかります。衝突が多い入力では時間が増えます。

`erase(key, false)`は次の要素を探さず削除し、`end()`を返します。有効なiteratorを使う`erase(it, false)`はO(1)です。省略時の`erase(key)`や`erase(it)`は次のiteratorを返すため、最悪O(capacity())です。

```cpp
HashSet<int> values;
values.insert(7);
if (auto it = values.find(7); it != values.end()) values.erase(it, false);
for (auto it = values.begin(); it != values.end();) it = values.erase(it);
```

`erase_if(pred)`は条件に合う要素を削除し、その数を返します。空集合ではO(1)、それ以外ではO(capacity())です。述語から表を変更しないでください。

`CompactSlot`に整数キーを保持し、`std::equal_to<Key>`または`std::equal_to<>`で比較する場合は、削除したキーの値を再登録の探索にも利用します。同値なキーの墓石に着いた時点で、その後ろに有効な同値キーはないため、その位置へ再登録できます。それ以外の型・比較器では登録済みの位置だけを比較します。削除したキーの参照先を保持し続ける必要はありません。削除したiteratorや参照が再び有効になるという意味ではありません。

## 容量・所有・反復子

`reserve(n)`はn要素分を予約し、`rehash(n)`はnバケット以上かつ現登録数に足りる容量へ再配置します。削除では自動縮小しません。`rehash(0)`で必要容量まで縮められます。

空集合の検索・`begin()`・集合比較は容量によらずO(1)です。`clear()`は容量を保ち、要素も墓石もなければ走査を省きます。コピーは独立した集合を作り、移動元は空になって再利用できます。

キーの書き換えは禁止です。再配置・clear・swap・ムーブでiteratorと要素参照が無効になり、eraseは対象だけを無効にします。`SKIP_RUNTIME_CHECKS`でiterator検査とその専用状態を除きます。検査でも、消して同じ位置へ再登録したiteratorなど全ての無効な使用は検出できません。容量オーバーフローの保護は維持します。

## 再配置の高速化

GCCのSSE2対応x86では、64バケット以上の表の再配置で登録状態を16個ずつ判定します。自明にコピー・破棄できるキーだけに適用し、ハッシュが例外を送出しても元の表を保持します。GCCのx86では新しい格納領域の一部を値の構築前に順に書き、最初のランダムな書き込みの負担も抑えます。

小さい表・非自明なキー・Clang・その他の対象環境では標準C++のループを使います。`SIRKIT_DISABLE_SIMD`でSIMD専用コードを除外できます。領域の事前準備は標準C++の処理として維持します。キーを探す探索手順は共通です。

## 反復子の状態走査

GCCのSSE2対応x86では、`optional`でキーを保持する場合、`begin()`・前後のiterator移動・次iteratorを返す削除で、先頭4バケットを逐次確認し、続く空き区間を16バケットずつ調べます。登録状態だけを読み、範囲外や破棄済みのキーには触れません。

`CompactSlot`、Clang、その他の対象環境、`SIRKIT_DISABLE_SIMD`指定時には通常の逐次走査を使います。iteratorや表の追加状態はありません。

## Verify

オンライン問題の入力に対する集合操作が、実際の出力に反映される問題を使います。

| 提出先 | 確認する操作 |
|---|---|
| [ABC344 C - A+B+C](https://atcoder.jp/contests/abc344/tasks/abc344_c) | 最大100万回の整数追加、重複、所属照会 |
| [ABC073 C - Write and Erase](https://atcoder.jp/contests/abc073/tasks/abc073_c) | 整数の追加・検索・削除・再登録・要素数 |
| [ABC164 C - gacha](https://atcoder.jp/contests/abc164/tasks/abc164_c) | 文字列の追加・重複・要素数 |

他のAPI・衝突・例外・コピー/移動・容量境界は、`Verify/current/tests/hash_set_test.cpp`と`hash_set_boundary_test.cpp`で独立参照に照合します。`hash_set_unit`は固定種のローカル検査です。速度比較は同じ問題・main・入出力・コンパイラ設定で行います。

使用例は`Examples/hash_set_example.cpp`、提出コードは`Verify/current/submissions/hash_set_membership.cpp`、`hash_set_toggle.cpp`、`hash_set_strings.cpp`です。
