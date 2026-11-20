# Z配列と文字列検索

各位置から始まる接尾辞と、列の先頭との共通接頭辞長を求めます。`sirkit::z_algorithm`は名前空間以外をACLの呼び出し形式に合わせています。

## API

### z_algorithm

```text
template<class T> std::vector<int> z_algorithm(const std::vector<T>& s);
inline std::vector<int> z_algorithm(const std::string& s);
template<class Sequence> requires requires(const Sequence& s) { s.size(); s[0] == s[0]; } std::vector<int> z_algorithm(const Sequence& s);
```

各位置からの接尾辞と列の先頭との共通接頭辞長を返します。非空ならz[0]=n、空なら空です。

**制約:** 列長はINT_MAX未満。size・添字アクセス・同値関係の==が使えること。

**計算量:** 比較・添字アクセスO(1)としてO(n+1)時間・領域。

### PrefixMatches

```text
template <class Pattern, class Text> std::vector<int> PrefixMatches(const Pattern& pattern, const Text& text);
```

textの各開始位置でpatternと一致する接頭辞長を返します。

**制約:** 各列長はINT_MAX未満で、size・添字アクセス・==が使えること。

**計算量:** text長n・pattern長mにO(n+m+1)時間・領域。

### ZMatches

```text
template <class Pattern, class Text> std::vector<int> ZMatches(const Pattern& pattern, const Text& text);
```

完全一致する開始位置を昇順で返します。空patternは全境界0..nです。

**制約:** 各列長はINT_MAX未満で、size・添字アクセス・==が使えること。

**計算量:** O(n+m+1)時間・領域。


nは入力列またはtext、mはpatternの長さです。比較・添字アクセスをO(1)とします。空列のZ配列は空、非空ならz[0]=nです。空patternは全境界0..nに一致します。特殊な区切り文字は不要です。

```cpp
const std::string text = "ababa", pattern = "aba";
auto z = z_algorithm(text);                     // {5,0,3,0,1}
auto positions = ZMatches(pattern,text);         // {0,2}
auto lengths = PrefixMatches(pattern,text);     // {3,0,3,0,1}
```

列長はINT_MAX未満、等値比較は一貫した同値関係が前提です。整数・バイト値の範囲は制限しません。入力を変更せず、返却vectorを独立して所有します。長さ上限の保護は検査省略時も残ります。

[完全な使用例](../Examples/z_algorithm_example.cpp) · [Library Checker](https://judge.yosupo.jp/problem/zalgorithm)
