// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/implicit_treap.cpp"
#endif
#include HEADER
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

// 要素別の操作とstd::vectorだけを参照にし、遅延状態を残したコピーを分割する。
struct AffineSum {
    struct S { std::int64_t sum, count; friend bool operator==(S, S) = default; };
    struct F { std::int64_t a, b; };
    static S op(S x, S y) { return {(x.sum + y.sum) % 101, x.count + y.count}; }
    static S e() { return {0, 0}; }
    static S mapping(F f, S x) { return {(f.a * x.sum + f.b * x.count) % 101, x.count}; }
    static F composition(F f, F g) { return {f.a * g.a % 101, (f.a * g.b + f.b) % 101}; }
    static F id() { return {1, 0}; }
};
struct Conjugation {
    struct S { std::uint32_t a, b; friend bool operator==(S, S) = default; };
    struct F { std::int64_t a, b, padding; };
    static S op(S x, S y) { return {x.a * y.a % 17, (x.b * y.a + y.b) % 17}; }
    static S e() { return {1, 0}; }
    static S mapping(F f, S x) {
        auto b = (f.a * x.b + (1 - std::int64_t(x.a)) * f.b) % 17;
        return {x.a, std::uint32_t((b + 17) % 17)};
    }
    static F composition(F f, F g) { return {f.a * g.a % 17, (f.a * g.b + f.b) % 17, 0}; }
    static F id() { return {1, 0, 0}; }
};

template<class A, bool Commutative>
std::uint64_t run() {
    using S = typename A::S;
    using F = typename A::F;
    using T = lazy_implicit_treap<S, A::op, A::e, F, A::mapping, A::composition, A::id, Commutative>;
    std::uint64_t checks = 0;
    auto fold = [](const std::vector<S>& v, int l, int r) {
        S answer = A::e();
        for (int i = l; i < r; ++i) answer = A::op(answer, v[i]);
        return answer;
    };
    auto verify = [&](T tree, const std::vector<S>& v) {
        assert(tree.size() == int(v.size()));
        assert(tree.all_prod() == fold(v, 0, int(v.size())));
        for (int l = 0; l <= int(v.size()); ++l) {
            assert(tree.prod(0, l) == fold(v, 0, l));
            assert(tree.prod(l, int(v.size())) == fold(v, l, int(v.size())));
            checks += 2;
        }
        assert(tree.to_vector() == v);
        checks += 3;
    };
    for (int n = 0; n <= 10; ++n) for (unsigned pattern = 0; pattern < 8; ++pattern) {
        std::vector<S> initial;
        for (int i = 0; i < n; ++i) initial.push_back({std::uint32_t(i + 1), 1});
        T base(initial);
        auto reference = initial;
        for (int step = 0; step < 3; ++step) {
            F f{}; f.a = step + 2; f.b = step * 3 + 1;
            base.apply(0, n, f);
            for (S& x : reference) x = A::mapping(f, x);
            if (pattern & (1U << step)) {
                base.reverse(0, n);
                std::reverse(reference.begin(), reference.end());
            }
        }
        verify(base, reference);
        for (int l = 0; l <= n; ++l) {
            { // 先頭・末尾・内部へ挿入し、切り離したスロットも再利用する。
                T t(base); auto v = reference;
                S fresh{3, 1}; t.insert_at(l, fresh); v.insert(v.begin() + l, fresh);
                verify(t, v); t.erase_at(l); v.erase(v.begin() + l); verify(t, v);
                t.insert_at(l, fresh); v.insert(v.begin() + l, fresh); verify(t, v);
            }
            for (int r = l; r <= n; ++r) {
                { T t(base); auto v = reference;
                  t.erase(l, r); v.erase(v.begin() + l, v.begin() + r); verify(t, v); }
                { T t(base); auto v = reference;
                  assert(t.erase_prod(l, r) == fold(v, l, r)); ++checks;
                  v.erase(v.begin() + l, v.begin() + r); verify(t, v); }
                { T t(base); auto v = reference;
                  t.reverse(l, r); std::reverse(v.begin() + l, v.begin() + r); verify(t, v); }
                for (int m = l; m <= r; ++m) {
                    T t(base); auto v = reference;
                    t.rotate(l, m, r); std::rotate(v.begin() + l, v.begin() + m, v.begin() + r);
                    verify(t, v);
                }
                for (int pos = 0; pos <= n; ++pos) {
                    T t(base); auto v = reference;
                    t.move_range(l, r, pos);
                    if (pos < l) std::rotate(v.begin() + pos, v.begin() + l, v.begin() + r);
                    else if (r < pos) std::rotate(v.begin() + l, v.begin() + r, v.begin() + pos);
                    verify(t, v);
                }
            }
        }
    }
    return checks;
}
int main() {
    auto a = run<AffineSum, true>();
    auto b = run<Conjugation, false>();
    std::cout << a << " commutative and " << b << " noncommutative split boundary checks passed\n";
}
