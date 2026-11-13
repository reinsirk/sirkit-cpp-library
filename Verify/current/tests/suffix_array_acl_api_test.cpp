// SPDX-License-Identifier: CC0-1.0
// 公開APIの型・呼出し形式と、追加の列型を独立な短列参照で確認する。
#ifndef SUFFIX_HEADER
#define SUFFIX_HEADER "../../../String/Suffix_Array.cpp"
#endif
#include SUFFIX_HEADER
#include SUFFIX_HEADER
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
using Result = std::vector<int>;
using StringSA = Result (*)(const std::string&);
using VectorSA = Result (*)(const std::vector<int>&);
using BoundedSA = Result (*)(const std::vector<int>&, int);
using StringLCP = Result (*)(const std::string&, const Result&);
using VectorLCP = Result (*)(const std::vector<int>&, const Result&);
static_assert(std::is_same_v<decltype(static_cast<StringSA>(&suffix_array)), StringSA>);
static_assert(std::is_same_v<decltype(static_cast<VectorSA>(&suffix_array<int>)), VectorSA>);
static_assert(std::is_same_v<decltype(static_cast<BoundedSA>(&suffix_array)), BoundedSA>);
static_assert(std::is_same_v<decltype(static_cast<StringLCP>(&lcp_array)), StringLCP>);
static_assert(std::is_same_v<decltype(static_cast<VectorLCP>(&lcp_array<int>)), VectorLCP>);
template<class S> concept HasLegacySA = requires(const S& s) { SuffixArray(s); };
template<class S> concept HasLegacyLCP = requires(const S& s, const Result& sa) { LCPArray(s, sa); };
static_assert(!HasLegacySA<std::string>);
static_assert(!HasLegacyLCP<std::string>);
std::uint64_t checks = 0;
void check(bool yes) { ++checks; if (!yes) std::abort(); }
template<class Exception, class F> void rejects(F f) {
    bool yes = false;
    try { f(); } catch (const Exception&) { yes = true; }
    check(yes);
}
template<class S> decltype(auto) value(const S& s, int i) {
    if constexpr (std::is_same_v<std::remove_cvref_t<S>, std::string> ||
                  std::is_same_v<std::remove_cvref_t<S>, std::string_view>)
        return static_cast<unsigned char>(s[i]);
    else return (s[i]);
}
template<class S> Result naive_sa(const S& s) {
    const int n = static_cast<int>(s.size());
    Result order(n); std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        while (a < n && b < n) {
            const auto& x = value(s, a); const auto& y = value(s, b);
            if (x < y) return true;
            if (y < x) return false;
            ++a; ++b;
        }
        return a == n && b != n;
    });
    return order;
}
template<class S> Result naive_lcp(const S& s, const Result& sa) {
    const int n = static_cast<int>(s.size());
    Result answer(n ? n - 1 : 0);
    for (int i = 0; i + 1 < n; ++i)
        while (answer[i] < n - sa[i] && answer[i] < n - sa[i + 1] &&
               s[sa[i] + answer[i]] == s[sa[i + 1] + answer[i]]) ++answer[i];
    return answer;
}
template<class T> void vector_case(const std::vector<T>& s) {
    const auto expected = naive_sa(s);
    check(suffix_array(s) == expected);
    check(suffix_array<T>(s) == expected);
    check(lcp_array(s, expected) == naive_lcp(s, expected));
    check(lcp_array<T>(s, expected) == naive_lcp(s, expected));
}
struct OnlyLess {
    int x;
    explicit OnlyLess(int v) : x(v) {}
    OnlyLess(const OnlyLess&) = delete;
    OnlyLess& operator=(const OnlyLess&) = delete;
    OnlyLess(OnlyLess&&) = default;
    OnlyLess& operator=(OnlyLess&&) = default;
    bool operator<(const OnlyLess& other) const { return x < other.x; }
    bool operator==(const OnlyLess&) const = delete;
    bool operator!=(const OnlyLess&) const = delete;
};
struct BooleanProxy {
    bool x;
    explicit operator bool() const { return x; }
    bool operator!() const = delete;
};
struct OnlyEquality {
    int x;
    explicit OnlyEquality(int v) : x(v) {}
    OnlyEquality(const OnlyEquality&) = delete;
    OnlyEquality& operator=(const OnlyEquality&) = delete;
    OnlyEquality(OnlyEquality&&) = default;
    OnlyEquality& operator=(OnlyEquality&&) = default;
    BooleanProxy operator==(const OnlyEquality& other) const { return {x == other.x}; }
};
struct Huge {
    std::size_t n;
    std::size_t size() const { return n; }
    int operator[](int) const { std::abort(); }
};
void generic_lcp_cases() {
    const std::vector<int> codes{2, 1, 2, 1, 2, 3, 1};
    const auto sa = naive_sa(codes), expected = naive_lcp(codes, sa);
    std::vector<OnlyEquality> eq;
    eq.reserve(codes.size());
    for (int x : codes) eq.emplace_back(x);
    check(lcp_array(eq, sa) == expected);
    check(lcp_array<OnlyEquality>(eq, sa) == expected);
    const std::array<OnlyEquality, 7> a{OnlyEquality(2), OnlyEquality(1), OnlyEquality(2),
        OnlyEquality(1), OnlyEquality(2), OnlyEquality(3), OnlyEquality(1)};
    check(lcp_array(a, sa) == expected);
    check(lcp_array(std::span<const int>(codes), sa) == expected);
    const std::array<int, 7> numbers{2, 1, 2, 1, 2, 3, 1};
    check(lcp_array(numbers, sa) == expected);
}
}
int main() {
    check(suffix_array({}).empty());
#ifndef SKIP_RUNTIME_CHECKS
    bool null_rejected=false;try{(void)suffix_array(static_cast<const char*>(nullptr));}catch(const std::invalid_argument&){null_rejected=true;}check(null_rejected);
#endif
    const StringSA string_sa = &suffix_array;
    const VectorSA vector_sa = &suffix_array<int>;
    const BoundedSA bounded_sa = &suffix_array;
    const StringLCP string_lcp = &lcp_array;
    const VectorLCP vector_lcp = &lcp_array<int>;
    const Result banana{5, 3, 1, 0, 4, 2};
    check(string_sa("banana") == banana);
    check(suffix_array("banana") == banana);
    check(string_lcp("banana", banana) == Result({1, 3, 0, 0, 2}));
    check(lcp_array("banana", banana) == Result({1, 3, 0, 0, 2}));
    const std::vector<int> sample{2, 1, 2, 1};
    const auto sample_sa = naive_sa(sample);
    check(vector_sa(sample) == sample_sa);
    check(bounded_sa(sample, 2) == sample_sa);
    check(suffix_array<int>({2, 1, 2, 1}) == sample_sa);
    check(vector_lcp(sample, sample_sa) == naive_lcp(sample, sample_sa));
    check(lcp_array<int>({2, 1, 2, 1}, sample_sa) == naive_lcp(sample, sample_sa));
    for (const std::string s : {std::string{}, std::string("a"), std::string("aa"),
            std::string("abba"), std::string({'\0', '\xff', '\x80', '\0', '\xff'}),
            std::string("mississippi")}) {
        const auto saved = s; const auto sa = naive_sa(s), lcp = naive_lcp(s, sa);
        const std::string_view view(s);
        check(suffix_array(s) == sa);
        check(suffix_array(view) == sa);
        check(lcp_array(s, sa) == lcp);
        check(lcp_array(view, sa) == lcp);
        check(s == saved);
    }
    vector_case(std::vector<int>{});
    vector_case(std::vector<char>{'\xff', '\0', '\x80', '\xff'});
    vector_case(std::vector<unsigned char>{255, 0, 128, 255});
    vector_case(std::vector<bool>{});
    vector_case(std::vector<bool>{false});
    vector_case(std::vector<bool>{true});
    vector_case(std::vector<bool>{false, true});
    vector_case(std::vector<bool>{true, false});
    vector_case(std::vector<bool>{false, false});
    vector_case(std::vector<bool>{true, true});
    vector_case(std::vector<bool>{true, false, true, false, true});
    vector_case(std::vector<std::int64_t>{INT64_MIN, INT64_MAX, -1, INT64_MIN, 0});
    vector_case(std::vector<std::int64_t>{INT64_MIN, INT64_MAX, -1, 0});
    vector_case(std::vector<std::uint64_t>{UINT64_MAX, 0, UINT64_MAX - 1, 0});
    vector_case(std::vector<std::uint64_t>{UINT64_MAX, 0, UINT64_MAX - 1});
    vector_case(std::vector<std::string>{"b", "a", "aa", "a", ""});
    vector_case(std::vector<std::string>{"b", "a", "aa", ""});
    vector_case(std::vector<double>{2.5, -3.0, 2.5, 0.0, -0.0});
    vector_case(std::vector<double>{2.5, -3.0, 0.0});
    std::vector<OnlyLess> ordered;
    for (int x : {2, 1, 2, 0, 1, 0}) ordered.emplace_back(x);
    check(suffix_array(ordered) == naive_sa(ordered));
    check(suffix_array<OnlyLess>(ordered) == naive_sa(ordered));
    std::vector<OnlyLess> distinct_ordered;
    for (int x : {2, -1, 3, 0}) distinct_ordered.emplace_back(x);
    check(suffix_array(distinct_ordered) == naive_sa(distinct_ordered));
    check(suffix_array<OnlyLess>(distinct_ordered) == naive_sa(distinct_ordered));
    generic_lcp_cases();
    check(lcp_array(std::string{}, {}) == Result{});
    check(lcp_array(std::vector<int>{}, {}) == Result{});
    check(SuffixArrayEqualRange(std::string_view("banana"), banana, std::string_view("ana")) == std::pair(1, 3));
    check(SuffixArrayEqualRange(std::string_view("banana"), banana, std::string_view{}) == std::pair(0, 6));
    for (std::size_t n : {static_cast<std::size_t>(std::numeric_limits<int>::max() - 1),
            static_cast<std::size_t>(std::numeric_limits<int>::max()),
            std::numeric_limits<std::size_t>::max()})
        rejects<std::length_error>([&] { (void)lcp_array(Huge{n}, {}); });
    rejects<std::length_error>([] { (void)suffix_array(std::vector<int>{}, -1); });
    rejects<std::length_error>([] { (void)suffix_array(std::vector<int>{}, std::numeric_limits<int>::max() - 1); });
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::out_of_range>([] { (void)suffix_array(std::vector<int>{-1}, 3); });
    rejects<std::out_of_range>([] { (void)suffix_array(std::vector<int>{4}, 3); });
    rejects<std::invalid_argument>([] { (void)lcp_array(std::string("a"), {}); });
    rejects<std::invalid_argument>([] { (void)lcp_array(std::string("aa"), {0, 0}); });
    rejects<std::invalid_argument>([] { (void)lcp_array(std::string("a"), {1}); });
#endif
    std::printf("suffix ACL API checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
