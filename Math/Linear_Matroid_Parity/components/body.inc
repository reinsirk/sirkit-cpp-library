#include <cstddef>
#include <span>
#include <stdexcept>

#ifndef SIRKIT_LINEAR_MATROID_PARITY_DEFINED
#define SIRKIT_LINEAR_MATROID_PARITY_DEFINED

template <class Field>
class LinearMatroidParity
{
public:
    struct Term
    {
        std::size_t index = 0;
        Field value{};
    };

    explicit LinearMatroidParity(std::size_t dimension)
        : matrix_(dimension, dimension)
    {
    }

    void add_pair(std::span<const Term> left, std::span<const Term> right, const Field &coefficient)
    {
        CheckIndices(left);
        CheckIndices(right);
        for (const auto &a : left)
        {
            const auto scaled = coefficient * a.value;
            for (const auto &b : right)
            {
                const auto product = scaled * b.value;
                matrix_(a.index, b.index) += product;
                matrix_(b.index, a.index) -= product;
            }
        }
    }

    [[nodiscard]] std::size_t sample_size() const
    {
        return MatrixRank(matrix_) / 2;
    }

private:
    void CheckIndices(std::span<const Term> terms) const
    {
        for (const auto &term : terms)
        {
            if (term.index >= matrix_.rows())
            {
                throw std::out_of_range("LinearMatroidParity: invalid coordinate");
            }
        }
    }

    Matrix<Field> matrix_;
};

#endif
