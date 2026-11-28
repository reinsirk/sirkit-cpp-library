#ifndef SIRKIT_TREE_BACKEDGE_ABSORPTION_DEFINED
#define SIRKIT_TREE_BACKEDGE_ABSORPTION_DEFINED

class TreeBackedgeAbsorption
{
public:
    explicit TreeBackedgeAbsorption(
        std::span<const int> parents,
        std::span<const std::pair<int, int>> backward_edges = {})
        : layout_(parents), children_(parents.size()), backward_edges_(parents.size())
    {
        if (std::ranges::count(parents, -1) != 1)
        {
            throw std::invalid_argument("TreeBackedgeAbsorption: expected one root");
        }
        for (const int vertex : layout_.order())
        {
            const int parent = parents[vertex];
            if (parent != -1)
            {
                children_[parent].push_back(vertex);
            }
        }
        for (const auto &[from, to] : backward_edges)
        {
            if (from == to || !layout_.contains(to, from) || children_[from].empty())
            {
                throw std::invalid_argument("TreeBackedgeAbsorption: invalid backward edge");
            }
            backward_edges_[from].push_back(to);
        }
    }

    template <class Field>
    [[nodiscard]] std::optional<Field> expected_steps() const
    {
        return Solve<Field>([](int)
                            {
                                return Field{1};
                            });
    }

    template <class Field>
    [[nodiscard]] std::optional<Field> expected_cost(std::span<const Field> costs) const
    {
        if (costs.size() != static_cast<std::size_t>(layout_.size()))
        {
            throw std::invalid_argument("TreeBackedgeAbsorption: wrong number of costs");
        }
        return Solve<Field>([costs](int vertex)
                            {
                                return costs[vertex];
                            });
    }

private:
    SubtreeIntervals layout_;
    std::vector<std::vector<int>> children_;
    std::vector<std::vector<int>> backward_edges_;

    template <class Field>
    struct Row
    {
        Field cost{};
        Field scale{1}, inverse_scale{1};
        std::map<int, Field> coefficients;
    };

    template <class Field, class Reward>
    std::optional<Field> Solve(Reward reward) const
    {
        auto order = layout_.order();
        if (order.empty())
        {
            return Field{};
        }
        std::vector<Row<Field>> rows(order.size());
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const int vertex = *it;
            const auto &children = children_[vertex];
            if (children.empty())
            {
                continue;
            }
            const auto &backward = backward_edges_[vertex];
            const Field degree(children.size() + backward.size());
            Field value = degree * reward(vertex);
            int largest = children.front();
            for (const int child : children)
            {
                value += rows[child].cost;
                if (rows[child].coefficients.size() > rows[largest].coefficients.size())
                {
                    largest = child;
                }
            }
            auto &row = rows[vertex];
            std::swap(row, rows[largest]);
            for (const int child : children)
            {
                auto &part = rows[child];
                const Field conversion = part.scale * row.inverse_scale;
                for (const auto &[ancestor, coefficient] : part.coefficients)
                {
                    row.coefficients[ancestor] += conversion * coefficient;
                }
                part.coefficients.clear();
            }
            for (const int ancestor : backward)
            {
                row.coefficients[ancestor] += row.inverse_scale;
            }
            Field pivot = degree;
            const auto self = row.coefficients.find(vertex);
            if (self != row.coefficients.end())
            {
                pivot = pivot - row.scale * self->second;
                row.coefficients.erase(self);
            }
            if (pivot == Field{})
            {
                return std::nullopt;
            }
            const Field reciprocal = pivot == Field{1} ? Field{1} : Field{1} / pivot;
            row.cost = value * reciprocal;
            row.scale = row.scale * reciprocal;
            row.inverse_scale = row.inverse_scale * pivot;
        }
        return rows[order.front()].cost;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
