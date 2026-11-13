# DynamicBitset

`DynamicBitset`は、実行時に長さを決められるbit列です。多数の真偽値を64bitずつまとめて扱い、集合演算や部分和DPに使えます。bit番号0が最下位です。[使用例](../Examples/dynamic_bitset_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `(n,value=false)` / `(string_view)` | n bitを同じ値 / 高位から並ぶ01文字列で初期化する。先頭の0も長さに含む。 |
| `test(pos)` / `operator[](pos)` | bitを取得する。非constの[]は代入できるproxyを返す。 |
| `set()` / `reset()` / `flip()` | 全bitを1 / 0 / 反転する。 |
| `set(pos,value=true)` / `reset(pos)` / `flip(pos)` | 1 bitを設定 / 0 / 反転する。bit操作は自身への参照を返す。 |
| `all()` / `any()` / `none()` | 全bitが1 / 1がある / 全bitが0か。空列はall・noneがtrue。 |
| `count()` / `count(l,r)` | 全体 / 半開区間[l,r)の1の数。 |
| `intersection_count(other)` / `intersection_parity(other)` | ANDの1の数 / 偶奇boolを、一時bitsetなしで返す。 |
| `&` / `\|` / `^` / `~` | AND / OR / XOR / 全bit反転。代入形も使える。2項演算は同じ長さが前提。 |
| `<<k` / `>>k` | 長さを保って論理shiftする。空いたbitは0。代入形も使える。 |
| `shift_or(k)` | `bits \|= bits << k`と同じ結果を一時配列なしで作る。 |
| `find_first()` / `find_last()` | 最初 / 最後の1の位置。なければsize()。 |
| `find_next(pos)` / `find_prev(pos)` | posより後の最初 / pos未満の最後の1。なければsize()。 |
| `resize(n,value=false)` / `reserve_bits(n)` | 残るbitを維持して長さ変更 / 容量予約。追加bitだけ指定値にする。 |
| `size()` / `empty()` / `clear()` / `swap(other)` | bit数 / 空か / 0 bitにする / 交換する。 |
| `to_string()` / `words()` | 高位からの01文字列 / 下位word順の読み取り専用spanを返す。 |

```cpp
DynamicBitset reachable(10001);
reachable.set(0);
for (std::size_t score : {2,3,5}) reachable.shift_or(score);
auto ways = reachable.count(); // 7: 0,2,3,5,7,8,10。
```

shift_orは元の各bitを1回だけ利用するため、上の例では各点数を一度だけ選ぶ部分和になります。k=0や長さ以上なら不変です。通常のshiftはkが長さ以上なら全0になります。縮小で消えたbitは再拡大しても復活しません。==と!=は長さも含めて比較します。

## 前提・計算量

1 bit操作とsize・emptyはO(1)。全bit操作・集合演算・shift・探索・コピーは最悪O(ceil(N/64))、count(l,r)はO((r-l)/64+1)、文字列との変換はO(N)です。resize・reserveの再確保は保持word数に比例します。move・swapはO(1)です。

`auto bit=bits[pos]`は値ではなくproxyのコピーです。値を保存するなら`bool bit=bits[pos]`とします。proxyはbool変換・代入・flip・boolとの&=・|=・^=に対応します。サイズ変更・代入・所有権移動後は以前のproxyやword spanを使わないでください。末尾wordの未使用bitは常に0です。

find_next(pos>=size())はsize()、find_prev(pos>size())はposをsize()へ丸めて探索します。コピーは独立、コピー代入・resize・reserveの確保失敗時は旧内容を維持します。ムーブ元は空として再利用できます。

番号・区間・2項演算の同長・01文字の検査は既定で有効です。`SKIP_RUNTIME_CHECKS`でこれらを除きますが、容量保護と巨大shiftの処理は維持します。対応CPUではPOPCNT・AVX2を使い、`SIRKIT_DISABLE_SIMD`で追加の専用実装とCPU判定を除きます。経路の詳細・測定は[Audit](../Audit/)を参照してください。

主Verifyは[Matrix Product (Mod 2)](https://judge.yosupo.jp/problem/matrix_product_mod_2)、AND個数は[ABC258 G](https://atcoder.jp/contests/abc258/tasks/abc258_g)、shift_orは[TDPC A](https://atcoder.jp/contests/tdpc/tasks/tdpc_contest)で補います。
