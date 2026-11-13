# Suffix Automaton

[SuffixAutomaton](../String/Suffix_Automaton.cpp)は文字列を末尾へ伸ばしながら、部分文字列の存在・出現回数・異なる個数・最長共通部分文字列を扱います。既定の`SuffixAutomaton<>`は英小文字、`SuffixAutomaton<256,0>`はNULを含む全バイトです。一般形は`<Alphabet,Base>`で、値を[Base,Base+Alphabet)に収めます。Unicodeの文字単位ではありません。[使用例](../Examples/suffix_automaton_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `SuffixAutomaton(string_view)` / `append(byte)` / `append(string_view)` | 文字列で構築 / 末尾へ追加する。 |
| `contains(pattern)` | 部分文字列として存在すればtrue。空は常にtrue。O(m)。 |
| `occurrences(pattern)` | 重なり込みの出現回数。空はn+1。 |
| `distinct_substring_count()` | 空を除く相異なる部分文字列数をuint64_tで返す。O(1)。 |
| `longest_common_substring(text)` | Match{first_start,second_start,length}を返す。O(m)。 |
| `size()` / `empty()` / `state_count()` | 元の文字列長 / 空か / 保存状態数。O(1)。 |
| `reserve(text_capacity)` / `reset(text={})` / `clear()` / `swap(other)` | 容量予約 / 新しい文字列へ置換 / 全解放して空へ / O(1)で交換。 |

```cpp
SuffixAutomaton<> automaton("ababa");
auto distinct = automaton.distinct_substring_count(); // 9。
auto count = automaton.occurrences("aba"); // 2。
auto match = automaton.longest_common_substring("zzbabab");
// first_start=1、second_start=2、length=4（"baba"）。
```

最長共通部分文字列の位置は自分側と引数側の順です。同長なら引数で先に終わるものを選び、長さ0なら位置も0です。元の文字列自体は保持せず、状態から自分側の位置を復元します。範囲外バイトの検索は不一致、最長共通部分文字列ではその位置で一致を区切ります。

## 計算量・所有と検査

同じ終了位置を持つ部分文字列を状態へまとめ、必要な場合に状態を複製します。1バイト追加は償却O(Alphabet)、1回の最悪はO(n+Alphabet)です。固定Alphabetなら償却O(1)。状態数は高々2n+1、領域はO(Alphabet*states+n+1)で、空なら根も確保しません。

出現回数はappend後の最初の一致する非空照会でO(n+states+m)のキャッシュを作り、以後O(m)です。確保失敗では元を保ちます。const照会もキャッシュを更新するため、同じ個体への並行キャッシュ照会は外部で同期してください。

コピーは独立、コピー代入・resetは失敗時に元を保ちます。moveはnoexcept・O(1)、移動元は空で再利用可です。1バイト追加は強い例外保証、文字列追加も確保・容量超過では不変ですが、途中の不正バイトでは処理済みの接頭辞を保持します。検査だけの全走査はしません。

`SKIP_RUNTIME_CHECKS`は追加バイトの検査を除き、照会の不一致判定とint32_t状態番号・長さ・容量の保護は維持します。構成・機能の数学的参考は[NyaanのSuffix Automaton](https://nyaannyaan.github.io/library/string/suffix-automaton.hpp.html)で、固定幅配列と出現回数キャッシュは独立に実装しています。
