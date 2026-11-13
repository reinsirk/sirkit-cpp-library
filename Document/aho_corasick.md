# Aho–Corasick法

[AhoCorasick](../String/Aho_Corasick.cpp)は、登録した複数の語を1本の文字列から同時に検索します。`AhoCorasick<>`は英小文字、`ByteAhoCorasick`は全256バイト、`AhoCorasick<Alphabet,First>`は連続したバイト範囲を扱います。[使用例](../Examples/aho_corasick_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `add(pattern)` | 語を登録し、登録順の0始まりIDを返す。空語・重複語も別ID。 |
| `build()` | 検索用の遷移を確定する。その後はclearまで語を追加しない。 |
| `count_total(text)` | 重なり・重複込みの総一致数を返す。O(text長)。 |
| `match_counts(text)` | 各IDの一致数をvectorで返す。O(text長+状態数+語数)。 |
| `for_each_match(text,visit)` | 一致ごとにvisit(id,begin,end)を呼ぶ。区間は半開。 |
| `find_all(text)` | AhoMatch{pattern,begin,end}のvectorを返す。 |
| `size()` / `empty()` / `state_count()` | 語数 / 語が空か / 根を含む状態数。 |
| `pattern_length(id)` / `pattern_state(id)` | 語長 / 終端状態。parentは元Trieの親、failureは最長の真の接尾辞の状態。 |
| `reserve_nodes(n)` / `reserve_patterns(n)` / `clear()` / `swap(other)` | 領域予約 / 空の未構築状態へ戻す / O(1)で交換。 |

```cpp
AhoCorasick<> automaton;
automaton.add("he");
automaton.add("she");
automaton.build();
auto total = automaton.count_total("shehe"); // 3。
auto counts = automaton.match_counts("shehe"); // {2,1}。
```

全遷移を状態ごとに持つため、構築時間・領域は状態数×Alphabetに比例します。for_each_matchとfind_allの時間はtext長と一致数に比例します。終了位置の昇順で通知し、同じ位置内のID順は保証しません。空語は開始前も含む全境界に一致します。

コピーは独立、move元は空の未構築状態です。clearは確保枠を保ちます。addの確保失敗・不正文字では未登録の接頭ノードが残る場合がありますが、登録済みの語は保ちます。buildの確保失敗は未構築のままで、再試行できます。

`SKIP_RUNTIME_CHECKS`は添字・文字範囲・構築状態の検査を除き、容量と総一致数の上限は維持します。Verifyは[Library Checker](https://judge.yosupo.jp/problem/aho_corasick)です。
