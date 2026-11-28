#include <span>

#ifndef SIRKIT_BIVARIATE_FORMAL_POWER_SERIES_DEFINED
#define SIRKIT_BIVARIATE_FORMAL_POWER_SERIES_DEFINED

namespace reinsirk::detail
{
constexpr std::uint32_t kBivariateFpsModulus = 998244353;

inline std::uint32_t bivariate_fps_reciprocal(std::uint32_t value) noexcept
{
    std::uint64_t result = 1;
    std::uint64_t power = value;
    for (std::uint32_t exponent = kBivariateFpsModulus - 2; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = result * power % kBivariateFpsModulus;
        }
        power = power * power % kBivariateFpsModulus;
    }
    return static_cast<std::uint32_t>(result);
}

struct BivariateFpsRectangle
{
    std::span<const std::uint32_t> coefficients;
    std::size_t rows;
    std::size_t columns;
    std::size_t stride;

    std::vector<std::uint32_t> pack(std::size_t width) const
    {
        std::vector<std::uint32_t> result((rows - 1) * width + columns);
        for (std::size_t row = 0; row < rows; ++row)
        {
            std::copy_n(coefficients.data() + row * stride, columns, result.data() + row * width);
        }
        return result;
    }
};

inline std::vector<std::uint32_t> extend_bivariate_fps_inverse(
    NTT<> &transform, BivariateFpsRectangle input, BivariateFpsRectangle inverse,
    std::size_t row_offset, std::size_t column_offset, std::size_t strip_rows, std::size_t strip_columns)
{
    // 行の間を空けて、yの指数が隣のx係数へ繰り上がるのを防ぐ。
    const auto first_width = input.columns + inverse.columns - 1;
    const auto product = transform.convolution_normalized(input.pack(first_width), inverse.pack(first_width));
    std::vector<std::uint32_t> error(strip_rows * strip_columns);
    for (std::size_t row = 0; row < strip_rows; ++row)
    {
        for (std::size_t column = 0; column < strip_columns; ++column)
        {
            const auto value = product[(row + row_offset) * first_width + column + column_offset];
            error[row * strip_columns + column] = value == 0 ? 0 : kBivariateFpsModulus - value;
        }
    }

    // 新しい帯の -A*B に旧逆数 B を掛け、増えた係数だけを求める。
    const auto second_width = inverse.columns + strip_columns - 1;
    const BivariateFpsRectangle error_view{error, strip_rows, strip_columns, strip_columns};
    const auto correction = transform.convolution_normalized(inverse.pack(second_width), error_view.pack(second_width));
    std::vector<std::uint32_t> result(input.rows * input.columns);
    for (std::size_t row = 0; row < inverse.rows; ++row)
    {
        std::copy_n(inverse.coefficients.data() + row * inverse.stride, inverse.columns, result.data() + row * input.columns);
    }
    for (std::size_t row = 0; row < strip_rows; ++row)
    {
        std::copy_n(correction.data() + row * second_width, strip_columns,
                    result.data() + (row + row_offset) * input.columns + column_offset);
    }
    return result;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<std::uint32_t> bivariate_fps_inverse(
    std::span<const std::uint32_t> coefficients, std::size_t rows, std::size_t columns)
{
    constexpr std::size_t kMaxArea = std::size_t{1} << 21;
    if (columns != 0 && rows > kMaxArea / columns)
    {
        throw std::length_error("bivariate_fps_inverse: area exceeds 2^21");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (coefficients.size() != rows * columns)
    {
        throw std::invalid_argument("bivariate_fps_inverse: coefficient count");
    }
#endif
    if (rows == 0 || columns == 0)
    {
        return {};
    }
    if (coefficients[0] == 0)
    {
        throw std::domain_error("bivariate_fps_inverse: zero constant");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (coefficients[0] >= reinsirk::detail::kBivariateFpsModulus)
    {
        throw std::invalid_argument("bivariate_fps_inverse: coefficient not normalized");
    }
#endif

    NTT<> transform;
    std::vector<std::uint32_t> inverse{reinsirk::detail::bivariate_fps_reciprocal(coefficients[0])};
    std::size_t current_rows = 1, current_columns = 1;
    while (current_rows < rows || current_columns < columns)
    {
        // 一方の精度だけを倍にし、他方で次数不足の積が現れないようにする。
        const bool extend_rows = current_rows < rows &&
                                 (current_columns == columns || current_rows * columns <= current_columns * rows);
        const auto next_rows = extend_rows ? std::min(rows, 2 * current_rows) : current_rows;
        const auto next_columns = extend_rows ? current_columns : std::min(columns, 2 * current_columns);
        const auto row_offset = extend_rows ? current_rows : 0;
        const auto column_offset = extend_rows ? 0 : current_columns;
        inverse = reinsirk::detail::extend_bivariate_fps_inverse(
            transform, {coefficients, next_rows, next_columns, columns},
            {inverse, current_rows, current_columns, current_columns}, row_offset, column_offset, next_rows - row_offset,
            next_columns - column_offset);
        current_rows = next_rows;
        current_columns = next_columns;
    }
    return inverse;
}
#endif
