// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TWO_SAT_TEST_SOURCE
#define SIRKIT_TWO_SAT_TEST_SOURCE "../../../Graph/Two_SAT.cpp"
#endif
#include SIRKIT_TWO_SAT_TEST_SOURCE
#include SIRKIT_TWO_SAT_TEST_SOURCE
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using Clause = std::tuple<int, bool, int, bool>;
static_assert(std::is_default_constructible_v<two_sat>);
static_assert(std::is_constructible_v<two_sat, int>);
static_assert(!std::is_convertible_v<int, two_sat>);
static_assert(std::is_same_v<decltype(&two_sat::add_clause), void(two_sat::*)(int,bool,int,bool)>);
static_assert(std::is_same_v<decltype(&two_sat::satisfiable), bool(two_sat::*)()>);
static_assert(std::is_same_v<decltype(&two_sat::answer), std::vector<bool>(two_sat::*)()>);
static_assert(std::is_same_v<decltype(&two_sat::value), bool(two_sat::*)(int) const>);
template<class T> concept HasSolve = requires(T& value) { value.solve(); };
template<class T> concept HasConstAnswer = requires(const T& value) { value.answer(); };
static_assert(!HasSolve<two_sat>);
static_assert(!HasConstAnswer<two_sat>);
std::uint64_t checks = 0, formulas = 0;
void check(bool condition) { ++checks; if (!condition) std::abort(); }
void empty_repeated(two_sat& formula, int n) {
    check(formula.size() == n && formula.clause_count() == 0);
    check(formula.answer().size() == static_cast<std::size_t>(n));
    for (int repeat = 0; repeat < 3; ++repeat) {
        check(formula.satisfiable());
        auto answer = formula.answer();
        check(answer.size() == static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) check(formula.value(i) == answer[i]);
        const auto saved = answer;
        if (n) answer[0] = !static_cast<bool>(answer[0]);
        check(formula.answer() == saved);
        formula.reserve_clauses(100);
        check(formula.clause_count() == 0 && formula.answer() == saved);
    }
}
void empty_cache_lifecycle() {
    for (int n : {0, 1, 7, 65}) {
        two_sat formula(n);
        for (int i = 0; i < n; ++i) formula.force(i, true);
        check(formula.satisfiable());
        for (bool value : formula.answer()) check(value);
        formula.reset(n);
#ifndef SKIP_RUNTIME_CHECKS
        if (n) {
            bool threw = false;
            try { (void)formula.value(0); } catch (const std::logic_error&) { threw = true; }
            check(threw);
        }
#endif
        empty_repeated(formula, n);
        auto copied = formula;
        empty_repeated(copied, n);
        auto moved = std::move(copied);
        empty_repeated(copied, 0);
        empty_repeated(moved, n);
        two_sat fixed(3);
        fixed.force(0, true); fixed.force(1, false);
        check(fixed.satisfiable());
        fixed.swap(moved);
        empty_repeated(fixed, n);
        check(moved.size() == 3 && moved.clause_count() == 2);
        check(moved.value(0) && !moved.value(1));
        moved.reset(n);
        empty_repeated(moved, n);
    }
}
void default_argument(two_sat formula = {}) {
    check(formula.size() == 0);
    check(formula.answer().empty());
    check(formula.satisfiable());
}
bool reference(int n, const std::vector<Clause>& clauses) {
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        bool ok = true;
        for (auto [i,a,j,b] : clauses)
            ok = ok && ((static_cast<bool>(mask >> i & 1u) == a) || (static_cast<bool>(mask >> j & 1u) == b));
        if (ok) return true;
    }
    return false;
}
void verify(two_sat& formula, int n, const std::vector<Clause>& clauses) {
    check(formula.size() == n);
    check(formula.clause_count() == clauses.size());
    // ACLでは未実行・UNSAT時もanswer()の長さがnで、内容だけが未規定。
    check(formula.answer().size() == static_cast<std::size_t>(n));
    const bool possible = reference(n, clauses);
    check(formula.satisfiable() == possible);
    auto assignment = formula.answer();
    check(assignment.size() == static_cast<std::size_t>(n));
    if (possible) {
        const two_sat& constant = formula;
        for (auto [i,a,j,b] : clauses) check((assignment[i] == a) || (assignment[j] == b));
        for (int i = 0; i < n; ++i) check(constant.value(i) == assignment[i]);
        if (n) {
            const bool original = assignment[0];
            assignment[0] = !original;
            check(formula.value(0) == original);
            check(formula.answer()[0] == original);
        }
    }
    check(formula.satisfiable() == possible);
    check(formula.answer().size() == static_cast<std::size_t>(n));
    ++formulas;
}
}

int main() {
    empty_cache_lifecycle();
    two_sat empty = {};
    default_argument();
    verify(empty, 0, {});
    for (int n = 1; n <= 2; ++n) {
        std::vector<Clause> all;
        for (int left = 0; left < 2*n; ++left)
            for (int right = left; right < 2*n; ++right)
                all.emplace_back(left/2, static_cast<bool>(left&1), right/2, static_cast<bool>(right&1));
        for (unsigned mask = 0; mask < (1u << all.size()); ++mask) {
            two_sat formula(n);
            check(formula.answer().size() == static_cast<std::size_t>(n));
            std::vector<Clause> clauses;
            for (unsigned i = 0; i < all.size(); ++i) if ((mask >> i) & 1u) {
                const auto [a,b,c,d] = all[i];
                formula.add_clause(a,b,c,d); clauses.push_back(all[i]);
            }
            verify(formula,n,clauses);
        }
    }
    std::mt19937_64 rng(20261104);
    for (int n = 1; n <= 9; ++n) for (int round = 0; round < 100; ++round) {
        two_sat formula(n);
        formula.reserve_clauses(100);
        std::vector<Clause> clauses;
        for (int stage = 0; stage < 6; ++stage) {
            const int i = static_cast<int>(rng()%n), j = static_cast<int>(rng()%n);
            const bool a = (rng()&1) != 0, b = (rng()&1) != 0;
            switch (stage%4) {
            case 0: formula.add_clause(i,a,j,b); clauses.emplace_back(i,a,j,b); break;
            case 1: formula.imply(i,a,j,b); clauses.emplace_back(i,!a,j,b); break;
            case 2: formula.force(i,a); clauses.emplace_back(i,a,i,a); break;
            default:
                formula.add_xor(i,j,a);
                clauses.emplace_back(i,true,j,a); clauses.emplace_back(i,false,j,!a);
            }
            verify(formula,n,clauses);
        }
        auto copied = formula;
        formula.reset();
        verify(formula,0,{});
        verify(copied,n,clauses);
        auto moved = std::move(copied);
        verify(copied,0,{});
        verify(moved,n,clauses);
        copied = moved;
        verify(copied,n,clauses);
        copied = std::move(moved);
        verify(moved,0,{});
        verify(copied,n,clauses);
        copied.swap(formula);
        verify(copied,0,{});
        verify(formula,n,clauses);
    }
    for (int bad : {-1, std::numeric_limits<int>::max()/2+1}) {
        bool threw = false;
        try { two_sat invalid(bad); } catch (const std::length_error&) { threw = true; }
        check(threw);
    }
#ifndef SKIP_RUNTIME_CHECKS
    {
        two_sat formula(2);
        check(formula.answer().size() == 2);
        bool threw = false;
        try { (void)formula.value(0); } catch (const std::logic_error&) { threw = true; }
        check(threw);
        formula.force(0,true); check(formula.satisfiable());
        formula.force(0,false); check(formula.answer().size() == 2);
        check(!formula.satisfiable()); check(formula.answer().size() == 2);
        threw = false;
        try { (void)formula.value(0); } catch (const std::logic_error&) { threw = true; }
        check(threw);
    }
#endif
    std::printf("TwoSAT ACL API formulas=%" PRIu64 " checks=%" PRIu64 "\n",formulas,checks);
}
