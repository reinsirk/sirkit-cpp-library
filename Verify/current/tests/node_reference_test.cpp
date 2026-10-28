// SPDX-License-Identifier: CC0-1.0
// アドレス列の参照モデルと、共有・例外・循環・長鎖の解放を照合する。
#ifndef SIRKIT_NODE_SOURCE
#define SIRKIT_NODE_SOURCE "../../../DataStructure/Node.cpp"
#endif
#include SIRKIT_NODE_SOURCE
#include SIRKIT_NODE_SOURCE

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
volatile int allocation_budget = -1;
volatile std::size_t allocation_calls = 0;
volatile std::size_t live_allocations = 0;
}

[[gnu::noinline]] void* operator new(std::size_t bytes) {
    if (allocation_budget == 0) throw std::bad_alloc();
    if (allocation_budget > 0) allocation_budget = allocation_budget - 1;
    if (void* p = std::malloc(bytes ? bytes : 1)) {
        allocation_calls = allocation_calls + 1;
        live_allocations = live_allocations + 1;
        return p;
    }
    throw std::bad_alloc();
}
[[gnu::noinline]] void* operator new[](std::size_t bytes) {
    return ::operator new(bytes);
}
[[gnu::noinline]] static void release_allocation(void* p) noexcept {
    if (p) live_allocations = live_allocations - 1;
    std::free(p);
}
void operator delete(void* p) noexcept { release_allocation(p); }
void operator delete[](void* p) noexcept { release_allocation(p); }
void operator delete(void* p, std::size_t) noexcept { release_allocation(p); }
void operator delete[](void* p, std::size_t) noexcept { release_allocation(p); }

static std::size_t checks = 0;
static void require(bool value, const char* label) {
    ++checks;
    if (!value) {
        std::cerr << "FAIL: " << label << '\n';
        std::exit(1);
    }
}

static void compare_chain(const std::shared_ptr<Node>& head,
                          const std::vector<Node*>& expected) {
    require(!has_cycle(head), "linear has_cycle");
    require(cycle_entry(head) == nullptr, "linear cycle_entry");
    require(chain_size(head) == expected.size(), "linear chain_size");
    require(chain_tail(head).get() == (expected.empty() ? nullptr : expected.back()), "linear tail");
    auto p = head;
    for (Node* node : expected) {
        require(p.get() == node, "linear pointer sequence");
        p = p->n;
    }
    require(!p, "linear exact termination");
}

static void boundary_tests() {
    std::shared_ptr<Node> empty;
    compare_chain(empty, {});
    require(!chain_nth(empty, 0), "empty nth");
    require(!chain_nth(empty, std::size_t(-1)), "empty max nth");
    require(!pop_front_node(empty), "empty pop");
    require(!break_cycle(empty), "empty break");
    require(!reverse_chain(empty), "empty reverse");
    require(!clone_chain(empty), "empty clone");
    clear_chain(empty);
    auto tail = make_node();
    require(!tail->n, "make_node default next");
    require(make_chain(0, tail) == tail, "zero make_chain shares tail");
    auto one = make_chain(1, tail);
    require(one != tail && one->n == tail, "make_chain attaches suffix");
    require(chain_nth(one, 0) == one && chain_nth(one, 1) == tail, "small nth");
    require(!chain_nth(one, 2), "short nth");
    auto old = one;
    require(pop_front_node(one), "nonempty pop");
    require(one == tail && old->n == tail, "pop shared old preserves next");
    push_front_node(one);
    require(one != tail && one->n == tail, "push shares prior head");

    Node a(tail);
    Node b(a);
    require(a.n == tail && b.n == tail, "Node copy shares next");
    Node c(std::move(b));
    require(c.n == tail && !b.n, "Node move transfers next");
    b = a;
    require(b.n == tail, "Node copy assignment");
    b = std::move(c);
    require(b.n == tail && !c.n, "Node move assignment");

    auto parent = make_node(make_chain(3));
    auto original = parent->n;
    push_front_node(parent->n);
    require(parent->n->n == original, "push member reference");
    auto pushed = parent->n;
    require(pop_front_node(parent->n), "pop member reference");
    require(parent->n == original && pushed->n == original, "pop member preserves shared node");
    clear_chain(parent->n);
    require(!parent->n && original->n, "clear member preserves external head");
    auto cycle = make_chain(3);
    chain_tail(cycle)->n = cycle;
    parent->n = cycle;
    require(break_cycle(parent->n), "break_cycle member reference");
    require(chain_size(parent->n) == 3, "member broken length");

    auto self = make_node();
    self->n = self;
    require(has_cycle(self) && cycle_entry(self) == self.get(), "self cycle");
    require(chain_nth(self, 7) == self, "finite nth on self cycle");
#ifndef SKIP_RUNTIME_CHECKS
    for (int kind = 0; kind < 3; ++kind) {
        bool caught = false;
        try {
            if (kind == 0) (void)chain_size(self);
            if (kind == 1) (void)chain_tail(self);
            if (kind == 2) (void)clone_chain(self);
        } catch (const std::invalid_argument&) { caught = true; }
        require(caught, "self cycle rejected");
    }
#endif
    require(break_cycle(self) && !self->n, "break self cycle");
    require(!break_cycle(self), "repeated cycle break");
}

static void random_tests() {
    std::mt19937_64 rng(0x8c394ca2376bd182ULL);
    for (int trial = 0; trial < 16000; ++trial) {
        const std::size_t count = rng() % 97;
        auto head = make_chain(count);
        std::vector<Node*> model;
        for (auto p = head; p; p = p->n) model.push_back(p.get());
        require(model.size() == count, "make_chain count");
        compare_chain(head, model);
        const auto idx = std::size_t(rng() % 110);
        require(chain_nth(head, idx).get() == (idx < count ? model[idx] : nullptr), "random nth");
        auto cloned = clone_chain(head);
        require(chain_size(cloned) == count && !has_cycle(cloned), "clone size");
        auto c = cloned;
        for (Node* p : model) {
            require(c && c.get() != p, "independent clone identity");
            c = c->n;
        }
        require(!c, "clone terminates");
        if (count) {
            auto observer = chain_nth(head, count / 2);
            Node* expected_next = count / 2 == 0 ? nullptr : model[count / 2 - 1];
            auto old = head;
            head = reverse_chain(head);
            std::reverse(model.begin(), model.end());
            compare_chain(head, model);
            require(observer->n.get() == expected_next, "shared reverse visible");
            require(!old->n, "old head now tail");
            require(chain_size(cloned) == count, "clone unaffected by original reverse");
        }
        if (count && trial % 2 == 0) {
            const std::size_t entry_idx = rng() % count;
            auto last = chain_tail(head);
            auto entry = chain_nth(head, entry_idx);
            last->n = entry;
            require(has_cycle(head), "random cyclic has_cycle");
            require(cycle_entry(head) == model[entry_idx], "random cycle entry");
            const std::size_t cyclic_idx = rng() % 700;
            const std::size_t pos = cyclic_idx < count ? cyclic_idx : entry_idx + (cyclic_idx - entry_idx) % (count - entry_idx);
            require(chain_nth(head, cyclic_idx).get() == model[pos], "finite cyclic nth");
#ifndef SKIP_RUNTIME_CHECKS
            if (trial % 32 == 0) {
                for (int kind = 0; kind < 3; ++kind) {
                    bool caught = false;
                    try {
                        if (kind == 0) (void)chain_size(head);
                        if (kind == 1) (void)chain_tail(head);
                        if (kind == 2) (void)clone_chain(head);
                    } catch (const std::invalid_argument&) { caught = true; }
                    require(caught, "prefix cycle rejected");
                }
            }
#endif
            require(break_cycle(head), "random break true");
            require(!last->n, "break removes closing edge");
            compare_chain(head, model);
            require(!break_cycle(head), "random break second false");
        }
        std::weak_ptr<Node> weak = head;
        clear_chain(head);
        require(!head && weak.expired(), "clear releases unique chain");
    }
}

static void lifetime_tests() {
    std::weak_ptr<Node> first, middle, last;
    {
        auto head = make_chain(1000000);
        first = head;
        middle = chain_nth(head, 500000);
        last = chain_nth(head, 999999);
    }
    require(first.expired() && middle.expired() && last.expired(), "million node automatic destruction");
    auto shared_suffix = make_chain(100000);
    std::weak_ptr<Node> suffix_weak = shared_suffix;
    std::weak_ptr<Node> suffix_end = chain_tail(shared_suffix);
    {
        auto prefix = make_chain(200000, shared_suffix);
        first = prefix;
        middle = chain_nth(prefix, 199999);
    }
    require(first.expired() && middle.expired(), "unique prefix released");
    require(!suffix_weak.expired() && chain_size(shared_suffix) == 100000, "shared suffix preserved");
    shared_suffix.reset();
    require(suffix_weak.expired() && suffix_end.expired(), "shared suffix eventually released");
    {
        auto head = make_chain(200000);
        auto suffix = chain_nth(head, 100000);
        first = head;
        last = chain_tail(suffix);
        clear_chain(head);
        require(!head && first.expired() && !last.expired(), "clear preserves shared suffix");
        require(chain_size(suffix) == 100000, "clear preserves shared suffix links");
    }
    require(last.expired(), "clear suffix eventual release");
}

static void cycle_boundaries() {
    const std::array<std::size_t, 11> sizes{0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63};
    for (const auto prefix : sizes) {
        for (const auto period : sizes) {
            if (period == 0) continue;
            auto head = make_chain(prefix + period);
            auto entry = chain_nth(head, prefix);
            auto tail = chain_tail(head);
            tail->n = entry;
            require(cycle_entry(head) == entry.get(), "boundary cycle entry");
            require(has_cycle(head), "boundary has_cycle");
            for (std::size_t index : {prefix, prefix + period, prefix + 3 * period})
                require(chain_nth(head, index) == entry, "boundary cyclic nth");
#ifndef SKIP_RUNTIME_CHECKS
            for (int operation = 0; operation < 3; ++operation) {
                bool caught = false;
                try {
                    if (operation == 0) (void)chain_size(head);
                    if (operation == 1) (void)chain_tail(head);
                    if (operation == 2) (void)clone_chain(head);
                } catch (const std::invalid_argument&) { caught = true; }
                require(caught, "boundary cyclic operation throws");
            }
#endif
            require(break_cycle(head), "boundary break cycle");
            require(!tail->n, "boundary closing edge removed");
            require(chain_size(head) == prefix + period, "boundary linear count");
        }
    }
}

static void allocation_failure_tests() {
    for (std::size_t length : {std::size_t{0}, std::size_t{1}, std::size_t{100}}) {
        auto head = make_chain(length);
        auto observer = head;
        const auto next = head ? head->n.get() : nullptr;
        const auto before_live = live_allocations;
        allocation_budget = 0;
        bool caught = false;
        try { push_front_node(head); }
        catch (const std::bad_alloc&) { caught = true; }
        allocation_budget = -1;
        require(caught, "push allocation failure throws");
        require(head == observer && (!head || head->n.get() == next), "failed push preserves head");
        require(live_allocations == before_live, "failed push leaks nothing");
        const auto before_calls = allocation_calls;
        push_front_node(head);
        require(allocation_calls == before_calls + 1, "push has one allocation");
        require(head->n == observer, "successful push preserves suffix");
    }
    auto parent = make_node(make_chain(3));
    auto original = parent->n;
    const auto before_live = live_allocations;
    allocation_budget = 0;
    bool caught = false;
    try { push_front_node(parent->n); }
    catch (const std::bad_alloc&) { caught = true; }
    allocation_budget = -1;
    require(caught && parent->n == original, "failed member push preserves original");
    require(live_allocations == before_live, "failed member push leaks nothing");
    for (int fail_after = 0; fail_after < 40; ++fail_after) {
        auto source = make_chain(40);
        auto suffix = make_chain(9);
        const auto live_before = live_allocations;
        allocation_budget = fail_after;
        caught = false;
        try { (void)clone_chain(source); }
        catch (const std::bad_alloc&) { caught = true; }
        allocation_budget = -1;
        require(caught && chain_size(source) == 40, "clone failure preserves source");
        require(live_allocations == live_before, "clone failure releases partial copy");
        allocation_budget = fail_after;
        caught = false;
        try { (void)make_chain(40, suffix); }
        catch (const std::bad_alloc&) { caught = true; }
        allocation_budget = -1;
        require(caught && chain_size(suffix) == 9, "make_chain failure preserves caller suffix");
        require(live_allocations == live_before, "make_chain failure releases partial prefix");
    }
}

static std::vector<std::weak_ptr<Node>> weak_observers(const std::shared_ptr<Node>& head) {
    std::vector<std::weak_ptr<Node>> result;
    for (auto p = head; p; p = p->n) result.emplace_back(p);
    return result;
}

static void require_expired(const std::vector<std::weak_ptr<Node>>& observers, const char* message) {
    for (const auto& observer : observers) require(observer.expired(), message);
}

static void additional_lifetime_tests() {
    int deleted = 0;
    {
        auto head = make_node(std::shared_ptr<Node>(nullptr, [&deleted](Node* pointer) {
            require(pointer == nullptr, "empty owning pointer deleter receives null");
            ++deleted;
        }));
        require(head->n.get() == nullptr && head->n.use_count() == 1, "empty pointer has control block");
        require(chain_size(head) == 1, "empty owning pointer terminates chain");
    }
    require(deleted == 1, "empty owning pointer deleter invoked once");
    {
        auto head = make_chain(300000);
        auto observers = weak_observers(head);
        require(observers.size() == 300000, "all-node weak observer count");
        head.reset();
        require_expired(observers, "all nodes destroyed with weak owners retained");
    }
    {
        Node owner(make_chain(300000));
        auto observers = weak_observers(owner.n);
        Node source(make_chain(4));
        owner = source;
        require(owner.n == source.n, "copy assignment shares replacement chain");
        require_expired(observers, "copy assignment releases long old chain");
        require(chain_size(owner.n) == 4, "copy assignment replacement intact");
    }
    {
        Node owner(make_chain(300000));
        auto observers = weak_observers(owner.n);
        Node source(make_chain(4));
        auto replacement = source.n;
        owner = std::move(source);
        require(owner.n == replacement && !source.n, "move assignment transfers replacement chain");
        require_expired(observers, "move assignment releases long old chain");
        require(chain_size(owner.n) == 4, "move assignment replacement intact");
    }
    {
        Node owner(make_chain(300000));
        auto observers = weak_observers(owner.n);
        Node source;
        owner = source;
        require(!owner.n, "copy empty assignment clears owner");
        require_expired(observers, "copy empty assignment releases long old chain");
    }
    {
        Node owner(make_chain(300000));
        auto observers = weak_observers(owner.n);
        Node source;
        owner = std::move(source);
        require(!owner.n, "move empty assignment clears owner");
        require_expired(observers, "move empty assignment releases long old chain");
    }
}

int main() {
    const auto before_live = live_allocations;
    boundary_tests();
    random_tests();
    cycle_boundaries();
    allocation_failure_tests();
    additional_lifetime_tests();
    lifetime_tests();
    require(live_allocations == before_live, "all allocation resources released");
    std::cout << "PASS checks=" << checks
              << " randomized_trials=16000 deep_nodes=1000000 allocation_failure_points=84 live_delta=0\n";
}
