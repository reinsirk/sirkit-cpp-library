// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/implicit_treap.cpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using SumTree = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;
struct Entry { int id; std::int64_t value; };
static std::uint64_t checks = 0;
static void require(bool ok) {
    ++checks;
    if (!ok) throw std::runtime_error("treap convenience reference mismatch");
}
static int position(const std::vector<Entry>& a, int label) {
    if (label < 0) return -1;
    for (int i = 0; i < static_cast<int>(a.size()); ++i)
        if (a[i].id == label) return i;
    return -1;
}
static void validate(SumTree& t, const std::vector<Entry>& a) {
    require(t.size() == static_cast<int>(a.size()));
    // 位置を先に問い合わせ、getによる遅延反転の解消に依存させない。
    for (int label = -2; label < 162; ++label) {
        const int p = position(a, label);
        require(t.position_of(label) == p);
        require(t.contains_id(label) == (p >= 0));
        if (p >= 0) {
            const auto x = t.get_by_id(label);
            require(x.sum == a[p].value && x.size == 1);
        }
    }
    require(t.position_of(std::numeric_limits<int>::max()) == -1);
    if (!a.empty()) {
        require(t.front().sum == a.front().value);
        require(t.back().sum == a.back().value);
    }
    std::int64_t sum = 0;
    for (int i = 0; i < t.size(); ++i) {
        require(t.get_id(i) == a[i].id);
        require(t.get(i).sum == a[i].value);
        sum += a[i].value;
    }
    require(t.all_prod().sum == sum && t.all_prod().size == static_cast<std::int64_t>(a.size()));
}
static void move_reference(std::vector<Entry>& a, int l, int r, int gap) {
    if (l == r || (l <= gap && gap <= r)) return;
    std::vector<Entry> block(a.begin() + l, a.begin() + r);
    a.erase(a.begin() + l, a.begin() + r);
    if (gap > r) gap -= r - l;
    a.insert(a.begin() + gap, block.begin(), block.end());
}
static std::pair<int,int> between(const std::vector<Entry>& a, int x, int y) {
    int l = position(a, x), r = position(a, y);
    require(l >= 0 && r >= 0);
    if (l > r) std::swap(l, r);
    if (l == r) return {l, l};
    return {l + 1, r};
}
static std::int64_t sum_range(const std::vector<Entry>& a, int l, int r) {
    std::int64_t result = 0;
    for (int p = l; p < r; ++p) result += a[p].value;
    return result;
}
#ifndef SKIP_RUNTIME_CHECKS
template<class F> static void rejected(F f) {
    bool caught = false;
    try { f(); } catch (const std::out_of_range&) { caught = true; }
    require(caught);
}
#endif
static void boundaries() {
    SumTree t;
    require(!t.erase_id(-1));
    require(!t.erase_id(std::numeric_limits<int>::max()));
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&] { (void)t.front(); });
    rejected([&] { (void)t.back(); });
    rejected([&] { (void)t.get_by_id(0); });
    rejected([&] { t.set_by_id(0, {7, 1}); });
    rejected([&] { t.insert_before(0, 1, {7, 1}); });
    rejected([&] { (void)t.prod_between(0, 0); });
    rejected([&] { (void)t.erase_between(0, 0); });
    rejected([&] { t.move_range(0, 0, 1); });
#endif
    t.insert_at_with_id(0, 4, {10, 1});
    t.insert_before(4, 9, {20, 1});
    t.insert_at(1, {30, 1});
    validate(t, {{9,20}, {-1,30}, {4,10}});
    require(t.prod_between(4, 9).sum == 30);
    require(t.prod_between(9, 4).size == 1);
    require(t.prod_between(4, 4).sum == 0);
    require(t.erase_between(4, 4).size == 0);
    require(t.erase_between(4, 9).sum == 30);
    require(t.erase_between(9, 4).sum == 0);
    t.set_by_id(4, {70, 1});
    require(t.get_by_id(4).sum == 70);
    require(t.erase_id(9));
    require(!t.erase_id(9));
    t.insert_before(4, 9, {99, 1});
    validate(t, {{9,99}, {4,70}});
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&] { (void)t.prod_between(4, 99); });
    rejected([&] { (void)t.erase_between(99, 4); });
    rejected([&] { (void)t.get_by_id(-1); });
    rejected([&] { t.set_by_id(99, {1, 1}); });
    rejected([&] { t.insert_before(99, 100, {1, 1}); });
    rejected([&] { t.move_range(-1, 1, 0); });
    rejected([&] { t.move_range(0, 3, 0); });
    rejected([&] { t.move_range(1, 0, 0); });
    rejected([&] { t.move_range(0, 2, -1); });
    rejected([&] { t.move_range(0, 2, 3); });
    validate(t, {{9,99}, {4,70}});
#endif
    lazy_implicit_treap<> plain;
    plain.push_back(4);
    plain.push_back(9);
    plain.push_back(2);
    require(plain.front() == 4 && plain.back() == 2);
    plain.move_range(0, 2, 3);
    require(plain.to_vector() == std::vector<std::int64_t>({2,4,9}));
}
static void exhaustive_moves() {
    for (int n = 0; n <= 8; ++n) {
        for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
            for (int gap = 0; gap <= n; ++gap) {
                SumTree t;
                std::vector<Entry> model;
                for (int i = 0; i < n; ++i) {
                    t.insert_at_with_id(i, i, {i, 1});
                    model.push_back({i,i+11});
                }
                t.apply(0, n, 11);
                t.reverse(0, n);
                std::reverse(model.begin(), model.end());
                t.move_range(l, r, gap);
                move_reference(model, l, r, gap);
                validate(t, model);
            }
        }
    }
}
static void random_reference(std::uint64_t seed) {
    SumTree t;
    std::vector<Entry> a;
    std::mt19937_64 rng(seed);
    for (int step = 0; step < 3200; ++step) {
        const int n = static_cast<int>(a.size());
        int l = static_cast<int>(rng() % (n+1)), r = static_cast<int>(rng() % (n+1));
        if (l > r) std::swap(l,r);
        const auto value = static_cast<std::int64_t>(rng()%101)-50;
        int label = static_cast<int>(rng()%160);
        while (position(a,label) >= 0) label = (label+1)%160;
        std::vector<int> known;
        for (const auto& x : a) if (x.id >= 0) known.push_back(x.id);
        switch (rng()%12) {
        case 0:
            if (n < 96) {t.insert_at_with_id(l,label,{value,1});a.insert(a.begin()+l,{label,value});}
            break;
        case 1:
            if (n < 96) {t.insert_at(l,{value,1});a.insert(a.begin()+l,{-1,value});}
            break;
        case 2:
            if (!known.empty() && n < 96) {
                const int anchor = known[rng()%known.size()], p = position(a,anchor);
                t.insert_before(anchor,label,{value,1});a.insert(a.begin()+p,{label,value});
            }
            break;
        case 3:
            if (!known.empty()) {
                const int anchor = known[rng()%known.size()], p = position(a,anchor);
                t.set_by_id(anchor,{value,1});a[p].value=value;
            }
            break;
        case 4: {
            const int candidate = static_cast<int>(rng()%164)-2, p = position(a,candidate);
            require(t.erase_id(candidate) == (p >= 0));
            if (p >= 0) a.erase(a.begin()+p);
            break;
        }
        case 5: case 6:
            if (!known.empty()) {
                const int x=known[rng()%known.size()],y=known[rng()%known.size()];
                const auto [first,last] = between(a,x,y);
                const auto sum = sum_range(a,first,last);
                require(t.prod_between(x,y).sum == sum);
                require(t.prod_between(y,x).size == last-first);
                if (rng()%2) {
                    const auto removed=t.erase_between(x,y);
                    require(removed.sum==sum && removed.size==last-first);
                    a.erase(a.begin()+first,a.begin()+last);
                }
            }
            break;
        case 7:
            t.apply(l,r,value);for(int p=l;p<r;++p)a[p].value+=value;
            break;
        case 8:
            t.reverse(l,r);std::reverse(a.begin()+l,a.begin()+r);
            break;
        case 9: {
            const int gap=static_cast<int>(rng()%(n+1));
            t.move_range(l,r,gap);move_reference(a,l,r,gap);
            break;
        }
        case 10: {
            const int middle=l+static_cast<int>(rng()%(r-l+1));
            t.rotate(l,middle,r);std::rotate(a.begin()+l,a.begin()+middle,a.begin()+r);
            break;
        }
        case 11:
            if (step%113==0) {t.clear();a.clear();}
            break;
        }
        if (step%9==0) validate(t,a);
    }
    validate(t,a);
}
static std::string concatenate(std::string a,std::string b){return a+b;}
static std::string empty_text(){return {};}
static std::string unchanged(int,std::string a){return a;}
static int compose(int,int){return 0;}
static int identity(){return 0;}
using TextTree=indexed_implicit_treap<std::string,concatenate,empty_text,int,unchanged,compose,identity>;
static void noncommutative() {
    TextTree t;
    for(int i=0;i<7;++i)t.insert_at_with_id(i,i,std::string(1,static_cast<char>('a'+i)));
    t.reverse(0,7); // gfedcba
    t.move_range(1,4,7); // gcbafed
    require(t.get_by_id(5)=="f" && t.front()=="g" && t.back()=="d");
    require(t.prod_between(0,6)=="cb");
    require(t.prod_between(6,0)=="cb");
    t.set_by_id(1,"B"); // gcBafed
    t.insert_before(0,10,"X"); // gcBXafed
    require(t.erase_between(5,2)=="BXa"); // gcfed
    require(!t.contains_id(10) && !t.contains_id(0));
    require(t.all_prod()=="gcfed");
    require(t.erase_id(6));
    require(t.all_prod()=="cfed");
}
int main() {
    boundaries();
    exhaustive_moves();
    for(std::uint64_t seed:{1,19,151,57623})random_reference(seed);
    noncommutative();
    std::cout<<"{\"checks\":"<<checks<<",\"status\":\"PASS\"}\n";
}
