#ifndef SIRKIT_DSU_ON_TREE_DEFINED
#define SIRKIT_DSU_ON_TREE_DEFINED

class DsuOnTree
{
public:
    explicit DsuOnTree(std::span<const int> parents)
        : DsuOnTree(SubtreeIntervals(parents))
    {
    }

    explicit DsuOnTree(SubtreeIntervals intervals)
        : intervals_(std::move(intervals)),
          heavy_(intervals_.size(), -1),
          keep_(intervals_.size())
    {
        const auto vertices = intervals_.order();
        schedule_.reserve(vertices.size());
        std::vector<int> pending;
        for (int i = 0; i < size();)
        {
            const int root = vertices[i];
            pending.push_back(root);
            i = intervals_.range(root).second;
        }
        while (!pending.empty())
        {
            const int v = pending.back();
            pending.pop_back();
            schedule_.push_back(v);
            const auto [first, last] = intervals_.range(v);
            int largest = 0;
            for (int i = first + 1; i < last;)
            {
                const int child = vertices[i];
                const int end = intervals_.range(child).second;
                if (end - i > largest)
                {
                    largest = end - i;
                    heavy_[v] = child;
                }
                i = end;
            }
            for (int i = first + 1; i < last;)
            {
                const int child = vertices[i];
                if (child != heavy_[v])
                {
                    pending.push_back(child);
                }
                i = intervals_.range(child).second;
            }
            if (const int child = heavy_[v]; child != -1)
            {
                keep_[child] = true;
                pending.push_back(child);
            }
        }
        std::ranges::reverse(schedule_);
    }

    [[nodiscard]] int size() const noexcept
    {
        return intervals_.size();
    }

    template <typename Add, typename Remove, typename Answer>
    void run(Add &&add, Remove &&remove, Answer &&answer) const
    {
        const auto vertices = intervals_.order();
        for (int vertex : schedule_)
        {
            const auto [first, last] = intervals_.range(vertex);
            if (const int heavy = heavy_[vertex]; heavy != -1)
            {
                const auto [begin, end] = intervals_.range(heavy);
                for (int i = first; i < begin; ++i)
                {
                    std::invoke(add, vertices[i]);
                }
                for (int i = end; i < last; ++i)
                {
                    std::invoke(add, vertices[i]);
                }
            }
            else
            {
                std::invoke(add, vertex);
            }
            std::invoke(answer, vertex);
            if (!keep_[vertex])
            {
                for (int i = first; i < last; ++i)
                {
                    std::invoke(remove, vertices[i]);
                }
            }
        }
    }

private:
    SubtreeIntervals intervals_;
    std::vector<int> heavy_;
    std::vector<bool> keep_;
    std::vector<int> schedule_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
