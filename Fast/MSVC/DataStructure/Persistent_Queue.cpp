// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Queue.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_queue.md
//
// API一覧
// PersistentQueue<T>(): 保存領域を作る。Version{} / empty_version()は空の版。
// push(version,value): 末尾へ追加した版を返す。償却O(1)時間・追加ノード1個。
// pop(version): 先頭を除いた版を返す。最悪O(log(P+1))時間・追加ノードなし。Pは累計push数。
// front(version): 先頭へのconst T&を返す。size(version) / empty(version)も各O(1)。
// push/pop後も元の版は使える。コピーしたキューは保存領域を共有する。
// 非空のVersionとfrontの参照は保存領域の所有者が残る間だけ有効。並行更新は外部同期が必要。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_QUEUE_DEFINED
#define SIRKIT_PERSISTENT_QUEUE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckPersistentQueueNodeCount(std::size_t count, std::size_t limit)
{
    if (count >= limit)
    {
        throw std::length_error("PersistentQueue: too many pushes");
    }
}
}

template <class T>
class PersistentQueue
{
    using Index = std::uint32_t;

    struct Node
    {
        Index parent, jump, depth;
        T value;
    };

    struct Storage
    {
        std::deque<Node> nodes;
    };

public:
    class Version
    {
        friend class PersistentQueue;

    public:
        Version() = default;

    private:
        Index front_ = 0, back_ = 0;
    };

    PersistentQueue() : storage_(std::make_shared<Storage>())
    {
    }

    [[nodiscard]] static Version empty_version() noexcept
    {
        return {};
    }

    [[nodiscard]] bool empty(Version version) const
    {
        CheckVersion(version);
        return version.front_ == 0;
    }

    [[nodiscard]] std::size_t size(Version version) const
    {
        CheckVersion(version);
        if (version.front_ == 0)
        {
            return 0;
        }
        return At(version.back_).depth - At(version.front_).depth + 1;
    }

    [[nodiscard]] const T &front(Version version) const
    {
        CheckNonempty(version);
        return At(version.front_).value;
    }

    [[nodiscard]] Version push(Version version, T value)
    {
        CheckVersion(version);
        if (!storage_)
        {
            storage_ = std::make_shared<Storage>();
        }

        auto &nodes = storage_->nodes;
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckPersistentQueueNodeCount(nodes.size(), std::numeric_limits<Index>::max());
        const auto id = static_cast<Index>(nodes.size() + 1);
        const auto parent = version.back_;
        const auto previous_jump = Jump(parent);
        const auto next_jump = Jump(previous_jump);
        const auto depth = Depth(parent);
        const auto previous_depth = Depth(previous_jump);
        // 先頭2ブロックが同長なら、新しいノードと合わせて長さ2s+1のブロックにする。
        const auto jump = depth - previous_depth == previous_depth - Depth(next_jump) ? next_jump : parent;
        nodes.emplace_back(parent, jump, depth + 1, std::move_if_noexcept(value));
        return MakeVersion(version.front_ == 0 ? id : version.front_, id);
    }

    [[nodiscard]] Version pop(Version version) const
    {
        CheckNonempty(version);
        if (version.front_ == version.back_)
        {
            return {};
        }

        const auto target_depth = At(version.front_).depth + 1;
        auto id = version.back_;
        while (At(id).depth > target_depth)
        {
            const auto &node = At(id);
            id = Depth(node.jump) >= target_depth ? node.jump : node.parent;
        }
        return MakeVersion(id, version.back_);
    }

private:
    std::shared_ptr<Storage> storage_;

    [[nodiscard]] const Node &At(Index index) const noexcept
    {
        return storage_->nodes[index - 1];
    }

    [[nodiscard]] Index Depth(Index index) const noexcept
    {
        return index == 0 ? 0 : At(index).depth;
    }

    [[nodiscard]] Index Jump(Index index) const noexcept
    {
        return index == 0 ? 0 : At(index).jump;
    }

    [[nodiscard]] Version MakeVersion(Index front, Index back) const noexcept
    {
        Version version;
        version.front_ = front;
        version.back_ = back;
        return version;
    }

    void CheckVersion(Version version) const
    {
        (void)version;
    }

    void CheckNonempty(Version version) const
    {
        CheckVersion(version);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
