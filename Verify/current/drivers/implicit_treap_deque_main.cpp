// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

// 列の左から順に一次関数を適用する、非可換な集約。
struct VerifyFunctionComposition {
    static constexpr std::uint64_t mod = 998244353;
    struct S { std::uint32_t a, b; };
    struct F {};
    static S op(S x, S y) {
        return {std::uint32_t(std::uint64_t(y.a) * x.a % mod),
                std::uint32_t((std::uint64_t(y.a) * x.b + y.b) % mod)};
    }
    static S e() { return {1, 0}; }
    static S mapping(F, S x) { return x; }
    static F composition(F, F) { return {}; }
    static F id() { return {}; }
};

int main() {
    int q; if (!(verify_in >> q)) return 0;
    using A = VerifyFunctionComposition;
    lazy_implicit_treap<A::S, A::op, A::e, A::F, A::mapping, A::composition, A::id> sequence;
    sequence.reserve(static_cast<std::size_t>(q));
    while (q--) {
        int type; verify_in >> type;
        if (type == 0 || type == 1) {
            A::S f; verify_in >> f.a >> f.b;
            if (type == 0) sequence.push_front(f);
            else sequence.push_back(f);
        } else if (type == 2) sequence.pop_front();
        else if (type == 3) sequence.pop_back();
        else {
            std::int64_t x; verify_in >> x;
            const auto f = sequence.all_prod();
            verify_out << (std::uint64_t(f.a) * x + f.b) % A::mod << '\n';
        }
    }
}
