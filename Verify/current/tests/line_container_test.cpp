// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/line-container.cpp"
#endif
#include HEADER
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace {
std::uint64_t checks = 0;
#if defined(__SIZEOF_INT128__)
__extension__ using IntegerReference = __int128;
#else
using IntegerReference = std::int64_t;
#endif

template<class Exception, class F> void throws(F operation) {
    bool caught = false;
    try { operation(); } catch (const Exception&) { caught = true; }
    assert(caught); ++checks;
}

template<class T, Objective objective>
void verify(const LineContainer<T, objective>& hull,
            const std::vector<std::pair<T,T>>& all, T x) {
    if (all.empty()) {
        assert(hull.empty() && hull.size() == 0);
        assert(!hull.query_optional(x)); assert(!hull.query_line_optional(x));
        assert(!hull.query_with_line_optional(x)); checks += 4;
        return;
    }
    using Reference = std::conditional_t<std::is_integral_v<T>, IntegerReference, long double>;
    auto evaluate = [x](std::pair<T,T> line) {
        return Reference(line.first) * Reference(x) + Reference(line.second);
    };
    Reference expected = evaluate(all.front());
    for (auto line : all) {
        if constexpr (objective == MINIMIZE) expected = std::min(expected, evaluate(line));
        else expected = std::max(expected, evaluate(line));
    }
    assert(!hull.empty() && hull.size() <= all.size()); ++checks;
    for (auto it = hull.begin(); it != hull.end(); ++it) {
        assert(std::find(all.begin(), all.end(), std::pair<T,T>{it->k,it->m}) != all.end());
        auto next = std::next(it);
        if (next != hull.end()) {
            if constexpr (objective == MAXIMIZE) {
                if constexpr (std::is_integral_v<T>) assert(it->k < next->k);
                else assert(it->k <= next->k);
            } else {
                if constexpr (std::is_integral_v<T>) assert(next->k < it->k);
                else assert(next->k <= it->k);
            }
            assert(it->p < next->p);
        }
        ++checks;
    }
    auto valid = [&](std::pair<T,T> line) {
        assert(std::find(all.begin(), all.end(), line) != all.end());
        assert(evaluate(line) == expected); checks += 2;
    };
    valid(hull.query_line(x)); valid(*hull.query_line_optional(x));
    const auto selected = hull.lower_bound(x);
    assert(selected != hull.end()); valid({selected->k,selected->m});
    if constexpr (std::is_integral_v<T>) {
        if (expected < Reference(std::numeric_limits<T>::min()) ||
            expected > Reference(std::numeric_limits<T>::max())) {
            throws<std::overflow_error>([&] { (void)hull.query(x); });
            throws<std::overflow_error>([&] { (void)hull.query_optional(x); });
            throws<std::overflow_error>([&] { (void)hull.query_with_line(x); });
            throws<std::overflow_error>([&] { (void)hull.query_with_line_optional(x); });
            return;
        }
    }
    assert(hull.query(x) == T(expected)); assert(hull.query_optional(x) == T(expected));
    auto result = hull.query_with_line(x); auto optional = *hull.query_with_line_optional(x);
    assert(result.value == T(expected) && optional.value == T(expected));
    valid({result.k,result.m}); valid({optional.k,optional.m}); checks += 3;
}

template<class T, Objective objective> void small_cases() {
    using Hull = LineContainer<T,objective>;
    std::mt19937_64 random(918423 + sizeof(T) + (objective == MINIMIZE));
    Hull empty; verify(empty, {}, T(0));
#ifndef SKIP_RUNTIME_CHECKS
    throws<std::out_of_range>([&] { (void)empty.query(T(0)); });
    throws<std::out_of_range>([&] { (void)empty.query_line(T(0)); });
    throws<std::out_of_range>([&] { (void)empty.query_with_line(T(0)); });
#endif
    for (int epoch = 0; epoch < 8; ++epoch) {
        Hull hull; std::vector<std::pair<T,T>> lines;
        for (int i = 0; i < 40; ++i) {
            T k = T(int(random() % 9) - 4), m = T(int(random() % 9) - 4);
            if constexpr (std::is_floating_point_v<T>) { k /= T(2); m /= T(2); }
            hull.add(k,m); lines.emplace_back(k,m);
            for (int query = -4; query <= 4; ++query) verify(hull, lines, T(query));
        }
        Hull copy(hull); copy.add(T(0), T(13)); auto more = lines; more.emplace_back(T(0),T(13));
        verify(copy, more, T(1)); verify(hull, lines, T(1));
        Hull assigned; assigned.add(T(3), T(2)); assigned = hull; verify(assigned,lines,T(-2));
        Hull moved(std::move(copy)); verify(moved,more,T(2)); copy.clear(); copy.add(T(2),T(1));
        verify(copy, {{T(2),T(1)}}, T(-3));
        assigned = std::move(moved); verify(assigned,more,T(-2)); moved.clear();
        hull.swap(copy); verify(hull,{{T(2),T(1)}},T(2)); verify(copy,lines,T(2));
        hull.clear(); verify(hull,{},T(0));
    }
    // 同点だけで選ばれる直線を含む、整数境界の隣接区間。
    Hull tie; std::vector<std::pair<T,T>> lines;
    for (auto line : {std::pair<T,T>{T(0),T(0)}, {T(2),T(-1)}, {T(1),T(0)}, {T(1),T(0)}}) {
        tie.add(line.first,line.second); lines.push_back(line);
        for (int x = -3; x <= 3; ++x) verify(tie,lines,T(x));
    }
}

template<class T, Objective objective> void integral_edges() {
    using Hull = LineContainer<T,objective>; const T lo=std::numeric_limits<T>::min(), hi=std::numeric_limits<T>::max();
    std::vector<std::pair<T,T>> lines; Hull hull;
    for (auto line : {std::pair<T,T>{T(0),lo}, {T(0),hi}, {T(-1),lo}, {T(1),hi},
                      {lo,T(0)}, {hi,T(0)}, {lo,hi}, {hi,lo}, {T(-1),T(0)}, {T(1),T(0)}}) {
        hull.add(line.first,line.second); lines.push_back(line);
        for (T x : {lo,T(lo+1),T(-1),T(0),T(1),T(hi-1),hi}) verify(hull,lines,x);
    }
#if defined(__SIZEOF_INT128__)
    if constexpr (sizeof(T) == 8) {
        std::mt19937_64 random(583719);
        Hull wide; lines.clear();
        for (int i=0;i<64;++i) {
            T k=std::bit_cast<std::int64_t>(random()), m=std::bit_cast<std::int64_t>(random());
            wide.add(k,m); lines.emplace_back(k,m);
            for(int j=0;j<5;++j) verify(wide,lines,std::bit_cast<std::int64_t>(random()));
        }
    }
#endif
}
void floating_edges() {
    using T=long double;
    assert(std::isinf(lc_div<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::min()))); ++checks;
    for (auto objective : {MINIMIZE,MAXIMIZE}) {
        if (objective==MINIMIZE) {
            MinLineContainer<T> hull;const T large=std::numeric_limits<T>::max()*T(0.75);
            hull.add(large,large); hull.add(-large,-large);
            assert(hull.query(T(0))==-large); assert(hull.query(T(-1))==0);
            const auto overflow = hull.query(T(2)); assert(std::isinf(overflow) && overflow < 0); checks+=3;
        } else {
            MaxLineContainer<T> hull;const T large=std::numeric_limits<T>::max()*T(0.75);
            hull.add(large,large); hull.add(-large,-large);
            assert(hull.query(T(0))==large); assert(hull.query(T(-1))==0);
            const auto overflow = hull.query(T(2)); assert(std::isinf(overflow) && overflow > 0); checks+=3;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    MinLineContainer<T> hull;hull.add(1,2);
    throws<std::invalid_argument>([&]{hull.add(std::numeric_limits<T>::infinity(),0);});
    throws<std::invalid_argument>([&]{(void)hull.query(std::numeric_limits<T>::infinity());});
    throws<std::invalid_argument>([&]{(void)hull.query_optional(std::numeric_limits<T>::quiet_NaN());});
#endif
}
}
int main() {
    small_cases<std::int8_t,MINIMIZE>();small_cases<std::int8_t,MAXIMIZE>();
    small_cases<std::int16_t,MINIMIZE>();small_cases<std::int16_t,MAXIMIZE>();
    small_cases<std::int32_t,MINIMIZE>();small_cases<std::int32_t,MAXIMIZE>();
    small_cases<std::int64_t,MINIMIZE>();small_cases<std::int64_t,MAXIMIZE>();
    small_cases<long double,MINIMIZE>();small_cases<long double,MAXIMIZE>();
    integral_edges<std::int8_t,MINIMIZE>();integral_edges<std::int8_t,MAXIMIZE>();
    integral_edges<std::int16_t,MINIMIZE>();integral_edges<std::int16_t,MAXIMIZE>();
    integral_edges<std::int32_t,MINIMIZE>();integral_edges<std::int32_t,MAXIMIZE>();
#if defined(__SIZEOF_INT128__)
    integral_edges<std::int64_t,MINIMIZE>();integral_edges<std::int64_t,MAXIMIZE>();
#endif
    using T=std::int64_t;
    throws<std::domain_error>([]{(void)lc_div<T>(1,0);});
    throws<std::overflow_error>([]{(void)lc_div<T>(std::numeric_limits<T>::min(),-1);});
    assert(lc_div<T>(-7,3)==-3 && lc_div<T>(7,-3)==-3 && lc_div<T>(-7,-3)==2);++checks;
    MinLineContainer<T> a;a.add(std::numeric_limits<T>::max(),std::numeric_limits<T>::min());
    assert(a.query(2)==std::numeric_limits<T>::max()-1);++checks;
    MinLineContainer<T> b;b.add(std::numeric_limits<T>::min(),std::numeric_limits<T>::min());assert(b.query(-1)==0);++checks;
    MaxLineContainer<T> c;c.add(std::numeric_limits<T>::max(),0);
    throws<std::overflow_error>([&]{(void)c.query(2);});
    floating_edges();
    std::cout<<checks<<" independent line-container API/ownership/overflow checks passed\n";
}
