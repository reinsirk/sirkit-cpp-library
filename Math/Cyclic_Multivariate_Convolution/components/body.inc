#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_CYCLIC_MULTIVARIATE_CONVOLUTION_DEFINED
#define SIRKIT_CYCLIC_MULTIVARIATE_CONVOLUTION_DEFINED
namespace detail
{
class CyclicPrimeField32
{
public:
    explicit CyclicPrimeField32(std::uint32_t modulus) noexcept : modulus_(modulus)
    {
    }

    [[nodiscard]] std::uint32_t modulus() const noexcept
    {
        return modulus_;
    }

    [[nodiscard]] std::uint32_t add(std::uint32_t a, std::uint32_t b) const noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<std::uint32_t>(sum >= modulus_ ? sum - modulus_ : sum);
    }

    [[nodiscard]] std::uint32_t subtract(std::uint32_t a, std::uint32_t b) const noexcept
    {
        return a >= b ? a - b : static_cast<std::uint32_t>(std::uint64_t{a} + modulus_ - b);
    }

    [[nodiscard]] std::uint32_t multiply(std::uint32_t a, std::uint32_t b) const noexcept
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % modulus_);
    }

    [[nodiscard]] std::uint32_t power(std::uint32_t a, std::uint32_t exponent) const noexcept
    {
        std::uint32_t result = 1;
        for (; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1U) != 0)
            {
                result = multiply(result, a);
            }
            a = multiply(a, a);
        }
        return result;
    }

private:
    std::uint32_t modulus_;
};

class CyclicAxisTransform
{
    using Coefficients = std::vector<std::uint32_t>;
    static constexpr std::size_t kNaiveLength = 16;

    struct Chirp
    {
        Coefficients factors;
        Coefficients kernel;
    };

public:
    CyclicAxisTransform(std::size_t length, std::uint32_t generator, CyclicPrimeField32 field)
        : field_(field), length_(length)
    {
        const auto modulus = field_.modulus();
        const auto root = field_.power(generator, static_cast<std::uint32_t>((modulus - 1) / length));
        const auto inverse = field_.power(root, modulus - 2);
        if (length > kNaiveLength && !std::has_single_bit(length))
        {
            forward_chirp_ = MakeChirp(root, inverse);
            inverse_chirp_ = MakeChirp(inverse, root);
        }
        else
        {
            roots_ = MakeRoots(root);
            inverse_roots_ = MakeRoots(inverse);
        }
    }

    void apply(Coefficients &values, std::size_t stride, bool inverse)
    {
        if (length_ == 2)
        {
            for (std::size_t base = 0; base < values.size(); base += 2 * stride)
            {
                for (std::size_t i = base; i < base + stride; ++i)
                {
                    const auto a = values[i], b = values[i + stride];
                    values[i] = field_.add(a, b);
                    values[i + stride] = field_.subtract(a, b);
                }
            }
            return;
        }
        Coefficients line(length_), scratch(length_);
        for (std::size_t base = 0; base < values.size(); base += stride * length_)
        {
            for (std::size_t offset = 0; offset < stride; ++offset)
            {
                const auto start = base + offset;
                for (std::size_t i = 0; i < length_; ++i)
                {
                    line[i] = values[start + i * stride];
                }
                if (length_ <= kNaiveLength)
                {
                    Naive(line, scratch, inverse ? inverse_roots_ : roots_);
                    line.swap(scratch);
                }
                else if (std::has_single_bit(length_))
                {
                    RadixTwo(line, inverse ? inverse_roots_ : roots_);
                }
                else
                {
                    Bluestein(line, inverse ? inverse_chirp_ : forward_chirp_);
                }
                for (std::size_t i = 0; i < length_; ++i)
                {
                    values[start + i * stride] = line[i];
                }
            }
        }
    }

private:
    CyclicPrimeField32 field_;
    std::size_t length_;
    Coefficients roots_, inverse_roots_;
    Chirp forward_chirp_, inverse_chirp_;
    CRTConvolution convolution_;

    [[nodiscard]] Coefficients MakeRoots(std::uint32_t root) const
    {
        Coefficients roots(length_, 1);
        for (std::size_t i = 1; i < length_; ++i)
        {
            roots[i] = field_.multiply(roots[i - 1], root);
        }
        return roots;
    }

    [[nodiscard]] Chirp MakeChirp(std::uint32_t root, std::uint32_t inverse) const
    {
        Chirp result{Coefficients(length_), Coefficients(2 * length_ - 1)};
        const auto center = length_ - 1;
        std::uint32_t positive = 1, negative = 1, step = 1, inverse_step = 1;
        for (std::size_t i = 0; i < length_; ++i)
        {
            result.kernel[center - i] = negative;
            positive = field_.multiply(positive, step);
            negative = field_.multiply(negative, inverse_step);
            result.kernel[center + i] = negative;
            result.factors[i] = positive;
            step = field_.multiply(step, root);
            inverse_step = field_.multiply(inverse_step, inverse);
        }
        return result;
    }

    void Naive(const Coefficients &input, Coefficients &output, const Coefficients &roots) const noexcept
    {
        for (std::size_t k = 0; k < length_; ++k)
        {
            std::uint32_t sum = 0;
            for (std::size_t j = 0; j < length_; ++j)
            {
                sum = field_.add(sum, field_.multiply(input[j], roots[j * k % length_]));
            }
            output[k] = sum;
        }
    }

    void RadixTwo(Coefficients &values, const Coefficients &roots) const noexcept
    {
        for (std::size_t i = 1, j = 0; i < length_; ++i)
        {
            auto step = length_ / 2;
            for (; (j & step) != 0; step /= 2)
            {
                j ^= step;
            }
            j ^= step;
            if (i < j)
            {
                std::swap(values[i], values[j]);
            }
        }
        for (std::size_t block = 2; block <= length_; block *= 2)
        {
            const auto half = block / 2;
            const auto step = length_ / block;
            for (std::size_t base = 0; base < length_; base += block)
            {
                for (std::size_t j = 0; j < half; ++j)
                {
                    const auto a = values[base + j];
                    const auto b = field_.multiply(values[base + half + j], roots[j * step]);
                    values[base + j] = field_.add(a, b);
                    values[base + half + j] = field_.subtract(a, b);
                }
            }
        }
    }

    void Bluestein(Coefficients &line, const Chirp &chirp)
    {
        // jk = j(j-1)/2 + k(k+1)/2 - (j-k)(j-k-1)/2。
        for (std::size_t i = 1; i < length_; ++i)
        {
            line[i] = field_.multiply(line[i], chirp.factors[i - 1]);
        }
        const auto linear = convolution_.modular(line, chirp.kernel, field_.modulus());
        for (std::size_t i = 0; i < length_; ++i)
        {
            line[i] = field_.multiply(linear[length_ - 1 + i], chirp.factors[i]);
        }
    }
};

inline std::vector<std::uint32_t> CyclicMultivariateConvolution(std::span<const std::size_t> dimensions,
                                                                std::span<const std::uint32_t> a,
                                                                std::span<const std::uint32_t> b,
                                                                std::uint32_t modulus)
{
    const auto generator = static_cast<std::uint32_t>(primitive_root(modulus));
    const CyclicPrimeField32 field(modulus);
    constexpr std::size_t kMaxSize = std::size_t{1} << 21;
    std::size_t size = 1;
    std::vector<std::size_t> strides;
    std::vector<CyclicAxisTransform> transforms;
    for (const auto length : dimensions)
    {
        if (length == 0)
        {
            throw std::invalid_argument("cyclic_multivariate_convolution: zero dimension");
        }
        if (length > kMaxSize / size)
        {
            throw std::length_error("cyclic_multivariate_convolution: total size exceeds 2^21");
        }
#ifndef SKIP_RUNTIME_CHECKS
        if ((modulus - 1) % length != 0)
        {
            throw std::invalid_argument("cyclic_multivariate_convolution: axis must divide prime-1");
        }
#endif
        if (length > 1)
        {
            strides.push_back(size);
            transforms.emplace_back(length, generator, field);
            size *= length;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (a.size() != size || b.size() != size)
    {
        throw std::invalid_argument("cyclic_multivariate_convolution: input counts do not match");
    }
#endif
    std::vector<std::uint32_t> left(size), right(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        left[i] = a[i] % modulus;
        right[i] = b[i] % modulus;
    }
    for (std::size_t axis = 0; axis < transforms.size(); ++axis)
    {
        transforms[axis].apply(left, strides[axis], false);
        transforms[axis].apply(right, strides[axis], false);
    }
    for (std::size_t i = 0; i < size; ++i)
    {
        left[i] = field.multiply(left[i], right[i]);
    }
    for (std::size_t axis = 0; axis < transforms.size(); ++axis)
    {
        transforms[axis].apply(left, strides[axis], true);
    }
    const auto inverse_size = field.power(static_cast<std::uint32_t>(size % modulus), modulus - 2);
    for (auto &entry : left)
    {
        entry = field.multiply(entry, inverse_size);
    }
    return left;
}
}

inline std::vector<std::uint32_t> cyclic_multivariate_convolution(std::span<const std::size_t> dimensions,
                                                                  std::span<const std::uint32_t> a,
                                                                  std::span<const std::uint32_t> b,
                                                                  std::uint32_t prime)
{
    return detail::CyclicMultivariateConvolution(dimensions, a, b, prime);
}
#endif
