# SPDX-License-Identifier: CC0-1.0
"""通常版の公開APIを保ち、HashMap/HashSetの解放を省く版を作る。"""
from pathlib import Path


try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def once(source: str, old: str, new: str) -> str:
    if source.count(old) != 1:
        raise ValueError("Hash高速版の差分を再確認してください: " + old[:100])
    return source.replace(old, new, 1)


def build(source: str, kind: str, root: Path) -> str:
    source = unwrap_source(source)
    source = once(source, "// API一覧\n", """// API一覧
// 速度優先版: 確保・構築成功と有効な引数が前提。例外時の状態復元は保証しない。
// eraseは非自明な登録値を破棄する。clear・表の破棄では登録値の破棄を省く。
// 再配置後の旧配列だけ解放し、最後の表と省略した値の資源は回収しない。
// 格納型のデストラクタの副作用・資源回収を必要としない処理で使う。
""")
    clear_comment = ("// clear(): 全要素を破棄し、確保済みの容量を再利用する。" if kind == "HashMap" else
                     "// clear(): O(capacity())で全要素を破棄し、確保済み容量を再利用する。")
    fast_clear = ("// clear(): 登録状態を消し容量を再利用する。値のデストラクタは呼ばない。" if kind == "HashMap" else
                  "// clear(): O(capacity())で登録状態を消し、容量を再利用する。値のデストラクタは呼ばない。")
    source = once(source, clear_comment, fast_clear)
    source = once(source, f"    {kind}(const {kind}&) = default;", f"""    {kind}(const {kind}& other) requires std::is_copy_constructible_v<value_type>
        : slots_(other.slots_.size()),state_(other.state_.size(),0),size_(other.size_),occupied_(other.occupied_),
          shift_(other.shift_),hash_(other.hash_),equal_(other.equal_) {{
        if(!state_.empty())std::copy_n(other.state_.data(),state_.size(),state_.data());
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))
        if(slots_.size()>=16) {{
            for(size_type base=0;base<slots_.size();base+=16) {{
                const auto states=_mm_loadu_si128(reinterpret_cast<const __m128i*>(state_.data()+base));
                unsigned mask;
                if constexpr(std::is_same_v<Slot,CompactSlot>)mask=static_cast<unsigned>(~_mm_movemask_epi8(_mm_cmpeq_epi8(states,_mm_setzero_si128())))&65535u;
                else mask=static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(states,_mm_set1_epi8(1))));
                while(mask) {{const auto i=base+std::countr_zero(mask);slots_[i].emplace(*other.slots_[i]);mask&=mask-1;}}
            }}
            return;
        }}
#endif
        for(size_type i=0;i<slots_.size();++i) {{
            if constexpr(std::is_same_v<Slot,CompactSlot>) {{
                if(state_[i])slots_[i].emplace(*other.slots_[i]);
            }} else if(state_[i]==1)slots_[i].emplace(*other.slots_[i]);
        }}
    }}""")
    start = source.index("    // 自明に", source.index("class " + kind + " {"))
    end = source.index("    static_assert(std::is_copy_constructible_v<value_type>", start)
    source = source[:start] + """    template<bool Compact>union SlotStorage {
        std::remove_cv_t<value_type> value;
        SlotStorage()noexcept{}
        ~SlotStorage() requires std::is_trivially_destructible_v<value_type> = default;
        ~SlotStorage()noexcept requires (!std::is_trivially_destructible_v<value_type>) {} // Fast契約: 登録値も破棄しない。
        SlotStorage(const SlotStorage&)=delete;
        SlotStorage(SlotStorage&&)=delete;
        value_type&operator*()noexcept{return value;}
        const value_type&operator*()const noexcept{return value;}
        value_type*operator->()noexcept{return std::addressof(value);}
        const value_type*operator->()const noexcept{return std::addressof(value);}
        template<class...Args>void emplace(Args&&...args){std::construct_at(std::addressof(value),std::forward<Args>(args)...);}
        void reset()noexcept{if constexpr(!std::is_trivially_destructible_v<value_type>)std::destroy_at(std::addressof(value));}
    };
    using CompactSlot=SlotStorage<true>;
    using Slot=std::conditional_t<
        std::is_nothrow_default_constructible_v<value_type>&&
        std::is_nothrow_move_constructible_v<value_type>&&
        std::is_trivially_copy_constructible_v<value_type>&&
        std::is_trivially_destructible_v<value_type>,CompactSlot,SlotStorage<false>>;
""" + source[end:]
    start = source.index("        if constexpr (std::is_nothrow_move_constructible_v<value_type> &&",
                         source.index("    void rebuild(size_type capacity)"))
    end = source.index("        slots_.swap(next);", start)
    key = "slots_[i]->first" if kind == "HashMap" else "*slots_[i]"
    source = source[:start] + f"""        for(size_type i=0;i<slots_.size();++i){{
            if(state_[i]!=1)continue;
            size_type p=bucket({key},shift);
            while(state[p])p=(p+1)&(capacity-1);
            if constexpr(std::is_move_constructible_v<value_type>)next[p].emplace(std::move(*slots_[i]));
            else next[p].emplace(*slots_[i]);
            state[p]=1;
        }}
""" + source[end:]
    source = once(source,
                  "        for (size_type i = 0; i < slots_.size(); ++i) if (state_[i] == 1) slots_[i].reset();\n", "")
    source = source.replace("std::vector<Slot>", "reinsirk::detail::FastHashReleasedArray<Slot>")
    source = source.replace("std::vector<std::uint8_t>", "reinsirk::detail::FastHashReleasedArray<std::uint8_t>")
    # 予約時にページへ触れる。値の寿命はemplaceで開始する。
    position = 0
    while (marker := source.find("// 値の構築前に領域を", position)) >= 0:
        begin = source.rfind("#if ", 0, marker)
        end = source.index("#endif", marker) + len("#endif")
        block = source[begin:end]
        if "defined(__GNUC__)" not in block:
            raise ValueError("Hashの事前書込み条件を確認してください")
        block = block[block.index("\n") + 1:block.rfind("#endif")]
        block = block.replace("if constexpr (std::is_same_v<Slot, CompactSlot>)", "")
        source = source[:begin] + block + source[end:]
        position = begin + len(block)
    # 登録状態の公開後に、退役した配列だけを回収する。
    for method in ("rebuild_simd", "rebuild"):
        begin = source.index("    void " + method + "(")
        opening = source.index("{", begin)
        end, depth = opening + 1, 1
        while depth:
            depth += (source[end] == "{") - (source[end] == "}")
            end += 1
        body = source[opening + 1:end - 1]
        at = body.rindex("invalidate();") + len("invalidate();")
        body = body[:at] + "\n        next.release_storage();state.release_storage();" + body[at:]
        source = source[:opening + 1] + body + source[end - 1:]
    helper = unwrap_source((root / "Fast/Source/Internal/Hash_Released_Array.cpp").read_text())
    helper = helper[helper.index("#include "):].rstrip() + "\n"
    at = source.index("#include ")
    return wrap_source(source[:at] + helper + "\n#if !defined(SIRKIT_DISABLE_SIMD) && defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))\n#include <emmintrin.h>\n#endif\n\n" + source[at:])
