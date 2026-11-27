// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Persistent_Queue.cpp"

#include <cassert>
#include <cstddef>
#include <deque>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
struct Payload
{
    int value;
    bool fail;

    explicit Payload(int value, bool fail = false) : value(value), fail(fail)
    {
    }

    Payload(const Payload &) = delete;

    Payload(Payload &&other) : value(other.value), fail(other.fail)
    {
        if (fail)
        {
            throw std::runtime_error("move failure");
        }
    }
};
}

int main()
{
    using Queue = sirkit::PersistentQueue<int>;
    using Version = Queue::Version;
    Queue queue;
    const Version empty;
    assert(queue.empty(empty) && queue.size(empty) == 0);
    const auto one = queue.push(empty, 7);
    const auto two = queue.push(one, 11);
    const auto three = queue.push(two, 13);
    const auto branch = queue.push(one, -17);
    assert(queue.size(three) == 3 && queue.size(branch) == 2);
    assert(queue.front(queue.pop(three)) == 11);
    const auto last = queue.pop(queue.pop(three));
    assert(queue.front(last) == 13);
    assert(queue.empty(queue.pop(last)));
    assert(queue.front(queue.pop(branch)) == -17);
    assert(queue.front(queue.push(queue.pop(one), 21)) == 21);

    auto copy = queue;
    const auto branched_again = copy.push(two, queue.front(three));
    assert(queue.front(queue.pop(queue.pop(branched_again))) == 7);
    assert(copy.front(three) == 7);
    auto moved = std::move(copy);
    assert(moved.front(two) == 7);
    const auto new_storage = copy.push(Queue::empty_version(), 73);
    assert(copy.front(new_storage) == 73);

    sirkit::PersistentQueue<Payload> payloads;
    const auto item = payloads.push(payloads.empty_version(), Payload(1));
    const auto *address = &payloads.front(item);
    try
    {
        (void)payloads.push(item, Payload(2, true));
        assert(false);
    }
    catch (const std::runtime_error &)
    {
    }
    auto chain = payloads.push(item, Payload(3));
    for (int i = 4; i <= 5000; ++i)
    {
        chain = payloads.push(chain, Payload(i));
    }
    assert(&payloads.front(chain) == address && payloads.front(item).value == 1);
    chain = payloads.pop(chain);
    for (int i = 3; i <= 5000; ++i)
    {
        assert(payloads.front(chain).value == i);
        chain = payloads.pop(chain);
    }
    assert(payloads.empty(chain));

    std::mt19937 rng(694828);
    for (int trial = 0; trial < 350; ++trial)
    {
        Queue candidate;
        std::vector<Version> versions(1);
        std::vector<std::deque<int>> old(1);
        for (int i = 0; i < 260; ++i)
        {
            // 新しい版と古い版の両方を繰り返し分岐させる。
            const auto id = rng() % 3 == 0 ? rng() % versions.size() : versions.size() - 1;
            const auto version = versions[id];
            auto expected = old[id];
            if (old[id].empty() || rng() % 4 != 0)
            {
                const auto value = static_cast<int>(rng() % 2001) - 1000;
                versions.push_back(candidate.push(version, value));
                expected.push_back(value);
            }
            else
            {
                assert(candidate.front(version) == expected.front());
                versions.push_back(candidate.pop(version));
                expected.pop_front();
            }
            old.push_back(expected);
            assert(candidate.size(versions.back()) == expected.size());
            assert(candidate.empty(versions.back()) == expected.empty());
            if (!expected.empty())
            {
                assert(candidate.front(versions.back()) == expected.front());
            }
            const auto old_id = rng() % versions.size();
            assert(candidate.size(versions[old_id]) == old[old_id].size());
            if (!old[old_id].empty())
            {
                assert(candidate.front(versions[old_id]) == old[old_id].front());
            }
        }
    }

    sirkit::detail::CheckPersistentQueueNodeCount(8, 9);
    try
    {
        sirkit::detail::CheckPersistentQueueNodeCount(9, 9);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (bool pop : {false, true})
    {
        try
        {
            if (pop)
            {
                (void)queue.pop(empty);
            }
            else
            {
                (void)queue.front(empty);
            }
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        (void)copy.size(one);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
