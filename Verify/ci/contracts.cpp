// SPDX-License-Identifier: CC0-1.0
// checked APIの正常系・例外型・失敗後の状態を小さい独立参照と照合する。
#if defined(SKIP_RUNTIME_CHECKS) || defined(NDEBUG)
#error "contracts require runtime checks and assertions"
#endif
#include "DataStructure/Fenwick_Tree.cpp"
#include "DataStructure/Segment_Tree.cpp"
#include "DataStructure/Lazy_Segment_Tree.cpp"
#include "DataStructure/union_find.cpp"
#include "Graph/Low_Link.cpp"
#include "String/RollingHash.cpp"
#if defined(SKIP_RUNTIME_CHECKS) || defined(NDEBUG)
#error "a dependency disabled runtime checks or assertions"
#endif
#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>

namespace {
struct FailureCase { const char* name; const char* exception; };
struct Result {
    const char* library;
    std::size_t normal_checks = 0;
    std::vector<FailureCase> failures;
    void require(bool value) {
        if (!value) throw std::runtime_error("normal result or post-failure state mismatch");
        ++normal_checks;
    }
    template<class Exception, class Function, class Postcondition>
    void expect(const char* name, const char* exception, Function function, Postcondition unchanged) {
        bool caught = false;
        try { function(); }
        catch (const Exception& error) {
            if (typeid(error) != typeid(Exception)) throw std::runtime_error("wrong exception subtype");
            caught = true;
        }
        catch (...) { throw std::runtime_error("wrong exception type"); }
        if (!caught) throw std::runtime_error("expected exception was not thrown");
        require(unchanged());
        failures.push_back({name, exception});
    }
};
using Integer = std::int64_t;
Integer sum_op(Integer left, Integer right) { return left + right; }
Integer zero() { return 0; }
struct Sum { Integer value; int length; };
Sum lazy_op(Sum left, Sum right) { return {left.value + right.value, left.length + right.length}; }
Sum lazy_zero() { return {0, 0}; }
Sum apply_add(Integer delta, Sum value) { return {value.value + delta * value.length, value.length}; }
Integer compose(Integer first, Integer second) { return first + second; }

Result fenwick_contract() {
    Result result{"DataStructure/Fenwick_Tree.cpp"};
    std::vector<Integer> values{3, 1, 4, 1, 5, 9, 2};
    sirkit::fenwick_tree<Integer> tree(values);
    tree.add(2, 7); values[2] += 7;
    tree.set(5, 6); values[5] = 6;
    for (int left = 0; left <= tree.size(); ++left)
        for (int right = left; right <= tree.size(); ++right)
            result.require(tree.sum(left, right) == std::accumulate(values.begin() + left, values.begin() + right, Integer{}));
    const auto prefix = std::accumulate(values.begin(), values.begin() + 3, Integer{});
    result.require(tree.lower_bound(prefix) == 2);
    result.require(tree.upper_bound(prefix) == 3);
    const auto unchanged = [&] { return tree.values() == values; };
    result.expect<std::out_of_range>("negative_point", "std::out_of_range", [&] { tree.add(-1, 2); }, unchanged);
    result.expect<std::out_of_range>("past_end_point", "std::out_of_range", [&] { tree.set(tree.size(), 2); }, unchanged);
    result.expect<std::out_of_range>("reversed_interval", "std::out_of_range", [&] { (void)tree.sum(3, 2); }, unchanged);
    result.expect<std::invalid_argument>("negative_reset", "std::invalid_argument", [&] { tree.reset(-1); }, unchanged);
    tree.add(0, 2); values[0] += 2; result.require(tree.values() == values);
    return result;
}
Result segment_contract() {
    Result result{"DataStructure/Segment_Tree.cpp"};
    std::vector<Integer> values{3, 1, 4, 1, 5, 9, 2};
    sirkit::segtree<Integer, sum_op, zero> tree(values);
    tree.set(2, 8); values[2] = 8;
    for (int left = 0; left <= tree.size(); ++left)
        for (int right = left; right <= tree.size(); ++right)
            result.require(tree.prod(left, right) == std::accumulate(values.begin() + left, values.begin() + right, Integer{}));
    const auto prefix = std::accumulate(values.begin(), values.begin() + 3, Integer{});
    result.require(tree.max_right(0, [=](Integer value) { return value <= prefix; }) == 3);
    const auto unchanged = [&] {
        for (int index = 0; index < tree.size(); ++index) if (tree.get(index) != values[index]) return false;
        return tree.all_prod() == std::accumulate(values.begin(), values.end(), Integer{});
    };
    result.expect<std::out_of_range>("negative_point", "std::out_of_range", [&] { tree.set(-1, 2); }, unchanged);
    result.expect<std::out_of_range>("past_end_interval", "std::out_of_range", [&] { (void)tree.prod(0, tree.size() + 1); }, unchanged);
    result.expect<std::invalid_argument>("predicate_rejects_identity", "std::invalid_argument", [&] { (void)tree.max_right(0, [](Integer) { return false; }); }, unchanged);
    tree.set(0, 11); values[0] = 11; result.require(unchanged());
    return result;
}
Result lazy_contract() {
    Result result{"DataStructure/Lazy_Segment_Tree.cpp"};
    std::vector<Integer> values{3, 1, 4, 1, 5, 9, 2};
    std::vector<Sum> initial; for (auto value : values) initial.push_back({value, 1});
    sirkit::lazy_segtree<Sum, lazy_op, lazy_zero, Integer, apply_add, compose, zero> tree(initial);
    tree.apply(1, 6, 3); for (int index = 1; index < 6; ++index) values[index] += 3;
    tree.apply(3, 2); values[3] += 2;
    for (int left = 0; left <= tree.size(); ++left)
        for (int right = left; right <= tree.size(); ++right) {
            const auto actual = tree.prod(left, right);
            result.require(actual.value == std::accumulate(values.begin() + left, values.begin() + right, Integer{}) && actual.length == right - left);
        }
    const auto prefix = std::accumulate(values.begin(), values.begin() + 3, Integer{});
    result.require(tree.max_right(0, [=](Sum value) { return value.value <= prefix; }) == 3);
    const auto unchanged = [&] {
        for (int index = 0; index < tree.size(); ++index) if (tree.get(index).value != values[index]) return false;
        return true;
    };
    result.expect<std::out_of_range>("negative_point", "std::out_of_range", [&] { tree.apply(-1, 7); }, unchanged);
    result.expect<std::out_of_range>("reversed_interval", "std::out_of_range", [&] { tree.apply(4, 2, 7); }, unchanged);
    result.expect<std::invalid_argument>("predicate_rejects_identity", "std::invalid_argument", [&] { (void)tree.max_right(0, [](Sum) { return false; }); }, unchanged);
    tree.apply(0, tree.size(), 1); for (auto& value : values) ++value; result.require(unchanged());
    return result;
}
Result union_contract() {
    Result result{"DataStructure/union_find.cpp"};
    sirkit::UnionFind groups(6);
    result.require(groups.merge(0, 1)); result.require(groups.merge(1, 2));
    result.require(!groups.merge(0, 2)); result.require(groups.merge(4, 5));
    result.require(groups.same(0, 2) && !groups.same(0, 3));
    result.require(groups.leader(0) == groups.leader(2));
    result.require(groups.size(1) == 3 && groups.component_count() == 3);
    const auto before = groups.groups();
    const auto unchanged = [&] { return groups.groups() == before && groups.component_count() == 3; };
    result.expect<std::out_of_range>("negative_vertex", "std::out_of_range", [&] { (void)groups.leader(-1); }, unchanged);
    result.expect<std::out_of_range>("past_end_merge", "std::out_of_range", [&] { (void)groups.merge(0, 6); }, unchanged);
    result.expect<std::length_error>("oversized_reset", "std::length_error", [&] { groups.reset(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1); }, unchanged);
    result.require(groups.merge(2, 3) && groups.size(0) == 4);
    return result;
}
Result low_link_contract() {
    Result result{"Graph/Low_Link.cpp"};
    sirkit::UndirectedLowLink graph(5);
    result.expect<std::logic_error>("query_before_build", "std::logic_error", [&] { (void)graph.bridges(); }, [&] { return graph.size() == 5 && graph.edge_count() == 0; });
    graph.add_edge(0, 1); graph.add_edge(1, 2); graph.add_edge(2, 0);
    const auto bridge = graph.add_edge(2, 3); graph.add_edge(3, 3);
    result.require(graph.build() == 2);
    result.require(graph.bridges() == std::vector<std::size_t>{bridge});
    result.require(graph.is_bridge(bridge));
    result.require(graph.articulation_points() == std::vector<int>({2}));
    result.require(graph.two_edge_component(0) == graph.two_edge_component(2));
    result.require(graph.two_edge_component(2) != graph.two_edge_component(3));
    const auto unchanged = [&] { return graph.edge_count() == 5 && graph.bridges() == std::vector<std::size_t>{bridge}; };
    result.expect<std::logic_error>("blocks_without_collection", "std::logic_error", [&] { (void)graph.biconnected_edge_components(); }, unchanged);
    result.expect<std::out_of_range>("past_end_vertex", "std::out_of_range", [&] { graph.add_edge(5, 0); }, unchanged);
    result.expect<std::out_of_range>("past_end_edge", "std::out_of_range", [&] { (void)graph.is_bridge(5); }, unchanged);
    result.require(graph.build(true) == 2);
    result.require(graph.biconnected_edge_components().size() == 3);
    result.require(graph.biconnected_vertex_components().size() == 4);
    graph.add_edge(3, 4);
    result.require(graph.build(true) == 1 && graph.bridges().size() == 2);
    return result;
}
Result rolling_contract() {
    Result result{"String/RollingHash.cpp"};
    const std::string text("ababa\0\xff", 7);
    sirkit::RollingHash hash(text, 10);
    for (std::size_t left = 0; left <= text.size(); ++left)
        for (std::size_t right = left; right <= text.size(); ++right) {
            std::uint64_t direct = 0;
            for (auto index = left; index < right; ++index) direct = direct * 10 + static_cast<unsigned char>(text[index]);
            result.require(hash.get(left, right) == direct);
        }
    result.require(hash.concat(hash.get(0, 3), hash.get(3, 7), 4) == hash.whole());
    result.require(hash.equal(0, 3, hash, 2, 5));
    const auto before = hash.whole();
    const auto unchanged = [&] { return hash.whole() == before && hash.size() == text.size(); };
    result.expect<std::out_of_range>("reversed_interval", "std::out_of_range", [&] { (void)hash.get(4, 3); }, unchanged);
    result.expect<std::out_of_range>("past_end_interval", "std::out_of_range", [&] { (void)hash.get(0, 8); }, unchanged);
    result.expect<std::out_of_range>("oversized_concatenation", "std::out_of_range", [&] { (void)hash.concat(0, 0, 8); }, unchanged);
    const sirkit::RollingHash other(text, 11);
    result.expect<std::invalid_argument>("different_bases", "std::invalid_argument", [&] { (void)hash.equal(0, 1, other, 0, 1); }, unchanged);
    hash.assign("aba"); result.require(hash.get(0, 3) == 10777);
    return result;
}
void print(const std::vector<Result>& results) {
    std::cout << "{\"format\":1,\"results\":[";
    bool first = true;
    for (const auto& result : results) {
        if (!first) std::cout << ','; first = false;
        std::cout << "{\"library\":\"" << result.library << "\",\"status\":\"PASS\",\"normal_checks\":" << result.normal_checks
                  << ",\"expected_failures\":" << result.failures.size() << ",\"cases\":[";
        bool first_case = true;
        for (const auto& item : result.failures) {
            if (!first_case) std::cout << ','; first_case = false;
            std::cout << "{\"name\":\"" << item.name << "\",\"expected_exception\":\"" << item.exception
                      << "\",\"status\":\"PASS\",\"post_failure_state_checked\":true}";
        }
        std::cout << "]}";
    }
    std::cout << "]}\n";
}
} // namespace
int main(int argc, char** argv) {
    try {
        // 契約検査自体が別の例外を成功扱いしないことを、単体テストから確認する。
        if (argc == 2 && std::string_view(argv[1]) == "--probe-wrong-exception") {
            Result probe{"self_test"};
            probe.expect<std::out_of_range>("wrong_type", "std::out_of_range", [] { throw std::invalid_argument("injected"); }, [] { return true; });
            return 2;
        }
        if (argc == 2 && std::string_view(argv[1]) == "--probe-exception-subtype") {
            Result probe{"self_test"};
            probe.expect<std::logic_error>("wrong_subtype", "std::logic_error", [] { throw std::out_of_range("injected"); }, [] { return true; });
            return 2;
        }
        if (argc != 1) throw std::invalid_argument("unexpected contract-test argument");
        print({fenwick_contract(), segment_contract(), lazy_contract(), union_contract(), low_link_contract(), rolling_contract()});
    }
    catch (const std::exception& error) { std::cerr << "contract failure: " << error.what() << '\n'; return 1; }
    catch (...) { std::cerr << "unexpected non-standard exception\n"; return 1; }
}
