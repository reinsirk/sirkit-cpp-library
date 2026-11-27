# べき乗塔の剰余

同じ底を右から重ねるべき乗塔を、指定した法で計算します。

```cpp
assert(sirkit::tetration_mod(2, 4, 1000) == 536);
```

## `tetration_mod`

```text
std::uint32_t tetration_mod(std::uint64_t base, std::uint64_t height, std::uint32_t mod);
```

`height` が0なら1の剰余を返します。たとえば高さ3は `base^(base^base)` です。0の0乗は1とします。法は正の32ビット符号なし整数で、底と法は互いに素である必要はありません。$O(\sqrt{mod}+\log^2(mod+1))$ 時間・$O(\log(mod+1))$ 領域です。

[使用例](../Examples/tetration_mod_example.cpp) ／ [Verify](../Verify/current/submissions/tetration_mod.cpp)
