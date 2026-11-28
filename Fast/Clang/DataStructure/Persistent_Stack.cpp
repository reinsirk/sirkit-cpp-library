// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Stack.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_stack.md
//
// API一覧
// PersistentStack<T>(): 空のスタックを作る。version_typeはsize_t、kEmptyは空の版。O(1)。
// emplace(version,args...): 指定版の末尾に値を構築した新しい版。元の版を保持する。償却O(1)。
// push(version,value): 値をコピーまたはムーブして追加する。償却O(1)。
// pop(version): 末尾を除いた既存の版。空ならkEmpty。O(1)。
// top(version): 非空の版の末尾をconst参照で返す。以降の追加でも参照は有効。O(1)。
// size(version)/empty(version): 版の要素数・空判定。O(1)。
// node_count(): 保存している全追加ノード数。O(1)。領域は追加回数Nに対してO(N+1)。
// 版は同じオブジェクトで取得した値。検査有効時は存在しない版と空版のtopでout_of_range。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_STACK_DEFINED
#define SIRKIT_PERSISTENT_STACK_DEFINED

template <class T>
class PersistentStack
{
public:
    using version_type = std::size_t;
    static constexpr version_type kEmpty = 0;

    PersistentStack() = default;
    PersistentStack(const PersistentStack &) requires std::constructible_from<T, const T &> = default;
    PersistentStack(PersistentStack &&) = default;

    PersistentStack &operator=(PersistentStack other) noexcept
    {
        nodes_.swap(other.nodes_);
        return *this;
    }

    template <class... Args>
        requires std::constructible_from<T, Args...>
    [[nodiscard]] version_type emplace(version_type version, Args &&...args)
    {
        const auto new_size = size(version) + 1;
        nodes_.emplace_back(version, new_size, std::forward<Args>(args)...);
        return nodes_.size();
    }

    [[nodiscard]] version_type push(version_type version, const T &value)
        requires std::constructible_from<T, const T &>
    {
        return emplace(version, value);
    }

    [[nodiscard]] version_type push(version_type version, T &&value)
        requires std::constructible_from<T, T &&>
    {
        return emplace(version, std::move(value));
    }

    [[nodiscard]] version_type pop(version_type version) const
    {
        CheckVersion(version);
        return version == kEmpty ? kEmpty : nodes_[version - 1].previous;
    }

    [[nodiscard]] const T &top(version_type version) const
    {
        CheckVersion(version);
        return nodes_[version - 1].value;
    }

    [[nodiscard]] std::size_t size(version_type version) const
    {
        CheckVersion(version);
        return version == kEmpty ? 0 : nodes_[version - 1].size;
    }

    [[nodiscard]] bool empty(version_type version) const
    {
        CheckVersion(version);
        return version == kEmpty;
    }

    [[nodiscard]] std::size_t node_count() const noexcept
    {
        return nodes_.size();
    }

private:
    struct Node
    {
        T value;
        version_type previous;
        std::size_t size;

        template <class... Args>
        explicit Node(version_type parent, std::size_t length, Args &&...args)
            : value(std::forward<Args>(args)...), previous(parent), size(length)
        {
        }
    };

    void CheckVersion([[maybe_unused]] version_type version) const
    {
    }

    std::deque<Node> nodes_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
