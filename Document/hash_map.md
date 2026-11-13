# HashMap

`HashMap<Key,Val,Hasher,KeyEqual>`はキーと値を管理するハッシュ表です。キーは順序不定で列挙され、キーと値のオブジェクトは表が所有します。[使用例](../Examples/hash_map_example.cpp)を参照してください。

検索・追加・次要素を探さないキー削除は期待償却O(1)です。ハッシュ計算は別途必要で、列キーでは列長に比例します。衝突が多い入力では時間が増えます。

## API

| API | 動作 |
| --- | --- |
| `HashMap()` / `HashMap(n,hash,equal)` | 空表 / n要素分を予約し、指定したハッシュ・同値判定で構築。 |
| `operator[](key)` | 値の参照を返す。未登録ならVal{}で追加。 |
| `find(key)` / `contains(key)` / `count(key)` | iterator（不在はend）/ 登録済みか / 0か1。追加しない。 |
| `at(key)` / `get_or_default(key,fallback)` | 値の参照（不在はout_of_range）/ 登録値またはfallbackのコピー。 |
| `insert({key,value})` / `emplace(key,value)` | 未登録なら追加し、対応するiteratorを返す。 |
| `try_emplace(key,args...)` | 未登録時だけ値を構築し、{iterator,追加したか}を返す。 |
| `insert_or_assign(key,value)` | 追加または上書きし、{iterator,追加したか}を返す。 |
| `erase(key,next=true)` / `erase(it,next=true)` | 削除し次iteratorを返す。falseなら次を探さずendを返す。 |
| `begin()` / `end()` | 要素を順序不定で列挙する。キーは変更不可。const版も使用可。 |
| `size()` / `empty()` | 登録数 / 空かをO(1)で返す。 |
| `reserve(n)` / `rehash(n)` | n要素分の容量を予約 / nバケット以上へ再配置し墓石を除く。必要な容量より縮めない。 |
| `capacity()` / `load_factor()` | バケット数 / 使用率をO(1)で返す。 |
| `clear()` / `swap(other)` | 容量を保持して全削除 / 内容とハッシュ・同値判定を交換。 |

```cpp
HashMap<int, int> counts;
counts[7] = 3;
auto value = counts.get_or_default(9, 0); // 0。キー9は追加しない。
counts.erase(7, false);                  // 次iteratorが不要ならfalse。
for (auto it = counts.begin(); it != counts.end();) it = counts.erase(it);
```

## 削除・列挙の計算量

未登録キーやend()の削除は何もしません。次iteratorを返す削除は空きバケットを走査するため、1回の最悪時間はO(capacity())です。返されたiteratorを順に使う全削除では、表全体の走査はO(capacity())に収まります。

次要素が不要ならeraseの第2引数をfalseにします。キー版は期待償却O(1)、有効なiterator版はO(1)で削除します。空表のbegin()はO(1)。削除で残る墓石はrehashやclearで除かれます。

## 所有と前提

コピーは独立した表を作ります。Key/Valはコピー可能、またはvalue_typeのムーブ構築がnoexceptであることが必要です。再配置・clear・swap・ムーブでiteratorと参照が無効になり、eraseでは対象だけが無効になります。再配置を伴う追加に同じ表の要素参照を渡す場合は、先にコピーしてください。

HasherとKeyEqualは、同値なキーに同じハッシュ値を返し、登録中に規則を変えないことが前提です。`GenericHash`は独立に使え、整数・tuple・列・std::hash対応型などを扱います。独自の同値関係には対応するHasherも用意します。

string_viewなどの借用キーでは、登録中に参照先の寿命と内容を保ちます。削除後は参照先を解放できます。swapはHasher/KeyEqualの交換が例外を投げないことを要求します。`SKIP_RUNTIME_CHECKS`でiteratorの所有元・世代・有効性の検査を省き、容量保護と要素の破棄は維持します。

GCCの対応環境では再配置や状態走査にSIMDなどを使います。`SIRKIT_DISABLE_SIMD`で専用命令を除きます。標準経路との使い分けや測定の詳細は[Audit](../Audit/)、集合版は[HashSet](hash_set.md)を参照してください。主Verifyは[Associative Array](../Verify/current/submissions/hash_map.cpp)です。
