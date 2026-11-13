#include <initializer_list>
#include <optional>

#ifndef SIRKIT_FORMAL_POWER_SERIES_DETAIL_DEFINED
#define SIRKIT_FORMAL_POWER_SERIES_DETAIL_DEFINED
namespace reinsirk::detail {
inline constexpr std::uint32_t fps_modulus = 998244353;
inline std::uint32_t fps_add(std::uint32_t a, std::uint32_t b) noexcept {
    const auto value = a + b; return value >= fps_modulus ? value - fps_modulus : value;
}
inline std::uint32_t fps_subtract(std::uint32_t a, std::uint32_t b) noexcept {
    return a >= b ? a - b : a + fps_modulus - b;
}
inline std::uint32_t fps_multiply(std::uint32_t a, std::uint32_t b) noexcept {
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % fps_modulus);
}
inline std::uint32_t fps_power(std::uint32_t value, std::uint64_t exponent) noexcept {
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1) {
        if (exponent & 1) result = fps_multiply(result, value);
        value = fps_multiply(value, value);
    }
    return result;
}
inline NTT<>& fps_engine() {
    static thread_local NTT<> engine;
    return engine;
}
inline std::vector<std::uint32_t>& fps_integer_inverses() {
    static thread_local std::vector<std::uint32_t> values;
    return values;
}
inline void fps_prepare_inverses(std::size_t n) {
    if (n >= fps_modulus) throw std::length_error("FormalPowerSeries: noninvertible integration index");
    auto& table = fps_integer_inverses();
    if (table.size() < 2) { table.resize(2); table[1] = 1; }
    if (n < table.size()) return;
    const auto first = table.size(); table.resize(n + 1);
    for (std::size_t i = first; i <= n; ++i)
        table[i] = fps_modulus - static_cast<std::uint32_t>(
            std::uint64_t{fps_modulus / static_cast<std::uint32_t>(i)} * table[fps_modulus % i] % fps_modulus);
}
inline void fps_require_precision(std::size_t n) {
    if (n > NTT<>::max_length() / 2) throw std::length_error("FormalPowerSeries: precision exceeds NTT budget");
}
inline std::optional<std::uint32_t> fps_scalar_sqrt(std::uint32_t value) noexcept {
    if (value == 0) return 0;
    if (fps_power(value, (fps_modulus - 1) / 2) != 1) return std::nullopt;
    constexpr std::uint32_t odd = 119;
    auto root = fps_power(value, (odd + 1) / 2), remainder = fps_power(value, odd);
    auto correction = fps_power(3, odd);
    unsigned remaining = 23;
    while (remainder != 1) {
        unsigned first = 0; auto probe = remainder;
        while (probe != 1 && first < remaining) { probe = fps_multiply(probe, probe); ++first; }
        if (first == remaining) return std::nullopt;
        const auto change = fps_power(correction, std::uint32_t{1} << (remaining - first - 1));
        root = fps_multiply(root, change); correction = fps_multiply(change, change);
        remainder = fps_multiply(remainder, correction); remaining = first;
    }
    return std::min(root, fps_modulus - root);
}
}
#endif

#ifndef SIRKIT_FORMAL_POWER_SERIES_DEFINED
#define SIRKIT_FORMAL_POWER_SERIES_DEFINED
class FormalPowerSeries {
    std::vector<std::uint32_t> coefficients_;
    struct Normalized {};
    FormalPowerSeries(std::vector<std::uint32_t> values, Normalized) noexcept : coefficients_(std::move(values)) {}
    template <std::integral T>
    static std::uint32_t normalize(T value) noexcept {
        static_assert(sizeof(T) <= sizeof(std::uint64_t));
        if constexpr (std::is_signed_v<T>) {
            const auto remainder = static_cast<std::int64_t>(value) % static_cast<std::int64_t>(modulus());
            return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus() : remainder);
        } else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) % modulus());
    }
    std::vector<std::uint32_t> short_prefix(std::size_t n) const {
        return {coefficients_.begin(), coefficients_.begin() + std::min(n, size())};
    }
    static FormalPowerSeries normalized(std::vector<std::uint32_t> values) noexcept {
        return FormalPowerSeries(std::move(values), Normalized{});
    }
public:
    static constexpr std::uint32_t modulus() noexcept { return reinsirk::detail::fps_modulus; }
    static constexpr std::size_t max_precision() noexcept { return NTT<>::max_length() / 2; }
    FormalPowerSeries() = default;
    explicit FormalPowerSeries(std::size_t n) : coefficients_(n) {}
    explicit FormalPowerSeries(std::vector<std::uint32_t> values) : coefficients_(std::move(values)) {
        for (auto& value : coefficients_) value %= modulus();
    }
    template <std::integral T>
    explicit FormalPowerSeries(const std::vector<T>& values) : coefficients_(values.size()) {
        for (std::size_t i = 0; i < values.size(); ++i) coefficients_[i] = normalize<T>(values[i]);
    }
    FormalPowerSeries(std::initializer_list<std::int64_t> values) : coefficients_(values.size()) {
        std::size_t at = 0; for (auto value : values) coefficients_[at++] = normalize(value);
    }
    [[nodiscard]] std::size_t size() const noexcept { return coefficients_.size(); }
    [[nodiscard]] bool empty() const noexcept { return coefficients_.empty(); }
    [[nodiscard]] const std::vector<std::uint32_t>& coefficients() const noexcept { return coefficients_; }
    [[nodiscard]] std::uint32_t operator[](std::size_t i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i >= size()) throw std::out_of_range("FormalPowerSeries: coefficient");
#endif
        return coefficients_[i];
    }
    [[nodiscard]] std::uint32_t get(std::size_t i) const noexcept { return i < size() ? coefficients_[i] : 0; }
    template <std::integral T>
    void set(std::size_t i, T value) {
#ifndef SKIP_RUNTIME_CHECKS
        if (i >= size()) throw std::out_of_range("FormalPowerSeries: coefficient");
#endif
        coefficients_[i] = normalize(value);
    }
    void resize(std::size_t n) { coefficients_.resize(n); }
    void trim() noexcept { while (!empty() && coefficients_.back() == 0) coefficients_.pop_back(); }
    [[nodiscard]] FormalPowerSeries prefix(std::size_t n) const {
        std::vector<std::uint32_t> result(n);
        std::copy_n(coefficients_.begin(), std::min(n, size()), result.begin());
        return normalized(std::move(result));
    }
    FormalPowerSeries& operator+=(const FormalPowerSeries& other) {
        if (other.size() > size()) coefficients_.resize(other.size());
        for (std::size_t i = 0; i < other.size(); ++i) coefficients_[i] = reinsirk::detail::fps_add(coefficients_[i], other.coefficients_[i]);
        return *this;
    }
    FormalPowerSeries& operator-=(const FormalPowerSeries& other) {
        if (other.size() > size()) coefficients_.resize(other.size());
        for (std::size_t i = 0; i < other.size(); ++i) coefficients_[i] = reinsirk::detail::fps_subtract(coefficients_[i], other.coefficients_[i]);
        return *this;
    }
    FormalPowerSeries& operator*=(const FormalPowerSeries& other) {
        auto result = reinsirk::detail::fps_engine().convolution_normalized(coefficients_, other.coefficients_);
        coefficients_.swap(result); return *this;
    }
    FormalPowerSeries& operator/=(const FormalPowerSeries& other) { *this = divmod(other).first; return *this; }
    FormalPowerSeries& operator%=(const FormalPowerSeries& other) { *this = divmod(other).second; return *this; }
    friend FormalPowerSeries operator+(FormalPowerSeries a, const FormalPowerSeries& b) { a += b; return a; }
    friend FormalPowerSeries operator-(FormalPowerSeries a, const FormalPowerSeries& b) { a -= b; return a; }
    friend FormalPowerSeries operator*(const FormalPowerSeries& a, const FormalPowerSeries& b) {
        return normalized(reinsirk::detail::fps_engine().convolution_normalized(a.coefficients_, b.coefficients_));
    }
    friend FormalPowerSeries operator/(const FormalPowerSeries& a, const FormalPowerSeries& b) { return a.divmod(b).first; }
    friend FormalPowerSeries operator%(const FormalPowerSeries& a, const FormalPowerSeries& b) { return a.divmod(b).second; }
    friend bool operator==(const FormalPowerSeries&, const FormalPowerSeries&) = default;
    template <std::integral T>
    [[nodiscard]] FormalPowerSeries scale(T scalar) const {
        auto result = coefficients_; const auto value = normalize(scalar);
        for (auto& coefficient : result) coefficient = reinsirk::detail::fps_multiply(coefficient, value);
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries derivative() const {
        if (empty()) return {};
        std::vector<std::uint32_t> result(size() - 1);
        for (std::size_t i = 1; i < size(); ++i)
            result[i - 1] = reinsirk::detail::fps_multiply(coefficients_[i], static_cast<std::uint32_t>(i % modulus()));
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries integral() const {
        if (empty()) return FormalPowerSeries{0};
        reinsirk::detail::fps_prepare_inverses(size());
        const auto& inverses = reinsirk::detail::fps_integer_inverses();
        std::vector<std::uint32_t> result(size() + 1);
        for (std::size_t i = 0; i < size(); ++i) result[i + 1] = reinsirk::detail::fps_multiply(coefficients_[i], inverses[i + 1]);
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries inverse(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
        if (get(0) == 0) throw std::domain_error("FormalPowerSeries: inverse of zero constant");
        std::vector<std::uint32_t> result{reinsirk::detail::fps_power(get(0), modulus() - 2)};
        auto& engine = reinsirk::detail::fps_engine();
        for (std::size_t known = 1; known < n;) {
            const auto target = std::min(n, known * 2);
            auto error = engine.convolution_normalized(short_prefix(target), result);
            error.resize(target);
            for (auto& coefficient : error) coefficient = coefficient == 0 ? 0 : modulus() - coefficient;
            error[0] = reinsirk::detail::fps_add(error[0], 2);
            auto next = engine.convolution_normalized(std::move(result), std::move(error)); next.resize(target);
            result.swap(next); known = target;
        }
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries inverse() const { return inverse(size()); }
    [[nodiscard]] FormalPowerSeries log(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
#ifndef SKIP_RUNTIME_CHECKS
        if (get(0) != 1) throw std::domain_error("FormalPowerSeries: logarithm constant must be one");
#endif
        auto differentiated = normalized(short_prefix(n)).derivative();
        auto result = reinsirk::detail::fps_engine().convolution_normalized(
            std::move(differentiated.coefficients_), inverse(n).coefficients_);
        result.resize(n - 1);
        return normalized(std::move(result)).integral();
    }
    [[nodiscard]] FormalPowerSeries log() const { return log(size()); }
    [[nodiscard]] FormalPowerSeries exp(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
#ifndef SKIP_RUNTIME_CHECKS
        if (get(0) != 0) throw std::domain_error("FormalPowerSeries: exponential constant must be zero");
#endif
        FormalPowerSeries result{1};
        for (std::size_t known = 1; known < n;) {
            const auto target = std::min(n, known * 2);
            auto correction = result.log(target).coefficients_;
            for (std::size_t i = 0; i < target; ++i) correction[i] = reinsirk::detail::fps_subtract(get(i), correction[i]);
            correction[0] = reinsirk::detail::fps_add(correction[0], 1);
            auto next = reinsirk::detail::fps_engine().convolution_normalized(std::move(result.coefficients_), std::move(correction));
            next.resize(target); result.coefficients_.swap(next); known = target;
        }
        return result;
    }
    [[nodiscard]] FormalPowerSeries exp() const { return exp(size()); }
    [[nodiscard]] FormalPowerSeries pow(std::uint64_t exponent, std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
        if (exponent == 0) { auto result = FormalPowerSeries(n); result.coefficients_[0] = 1; return result; }
        std::size_t leading = 0;
        while (leading < std::min(n, size()) && coefficients_[leading] == 0) ++leading;
        if (leading == std::min(n, size()) || (leading != 0 && exponent > (n - 1) / leading)) return FormalPowerSeries(n);
        const auto shift = leading == 0 ? 0 : leading * static_cast<std::size_t>(exponent);
        const auto remaining = n - shift;
        const auto first = coefficients_[leading], inverse_first = reinsirk::detail::fps_power(first, modulus() - 2);
        std::vector<std::uint32_t> unit(std::min(remaining, size() - leading));
        for (std::size_t i = 0; i < unit.size(); ++i) unit[i] = reinsirk::detail::fps_multiply(coefficients_[leading + i], inverse_first);
        auto logarithm = normalized(std::move(unit)).log(remaining);
        const auto reduced_exponent = static_cast<std::uint32_t>(exponent % modulus());
        for (auto& coefficient : logarithm.coefficients_) coefficient = reinsirk::detail::fps_multiply(coefficient, reduced_exponent);
        auto powered = logarithm.exp(remaining);
        const auto multiplier = reinsirk::detail::fps_power(first, exponent);
        std::vector<std::uint32_t> result(n);
        for (std::size_t i = 0; i < remaining; ++i) result[shift + i] = reinsirk::detail::fps_multiply(powered.coefficients_[i], multiplier);
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries pow(std::uint64_t exponent) const { return pow(exponent, size()); }
    [[nodiscard]] std::optional<FormalPowerSeries> sqrt(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return FormalPowerSeries{};
        std::size_t leading = 0;
        while (leading < std::min(n, size()) && coefficients_[leading] == 0) ++leading;
        if (leading == std::min(n, size())) return FormalPowerSeries(n);
        if (leading & 1) return std::nullopt;
        const auto root = reinsirk::detail::fps_scalar_sqrt(coefficients_[leading]);
        if (!root) return std::nullopt;
        const auto needed = n - leading;
        FormalPowerSeries result{static_cast<std::int64_t>(*root)};
        for (std::size_t known = 1; known < needed;) {
            const auto target = std::min(needed, known * 2);
            std::vector<std::uint32_t> part(std::min(target, size() - leading));
            std::copy_n(coefficients_.begin() + leading, part.size(), part.begin());
            auto ratio = reinsirk::detail::fps_engine().convolution_normalized(std::move(part), result.inverse(target).coefficients_);
            ratio.resize(target);
            for (std::size_t i = 0; i < target; ++i)
                ratio[i] = reinsirk::detail::fps_multiply(reinsirk::detail::fps_add(ratio[i], result.get(i)), (modulus() + 1) / 2);
            result.coefficients_.swap(ratio); known = target;
        }
        std::vector<std::uint32_t> shifted(n);
        std::copy(result.coefficients_.begin(), result.coefficients_.end(), shifted.begin() + leading / 2);
        return normalized(std::move(shifted));
    }
    [[nodiscard]] std::optional<FormalPowerSeries> sqrt() const { return sqrt(size()); }
    [[nodiscard]] std::pair<FormalPowerSeries, FormalPowerSeries> divmod(const FormalPowerSeries& divisor) const {
        auto a = *this, b = divisor; a.trim(); b.trim();
        if (b.empty()) throw std::domain_error("FormalPowerSeries: zero polynomial divisor");
        if (a.size() < b.size()) return {FormalPowerSeries{}, std::move(a)};
        const auto quotient_size = a.size() - b.size() + 1;
        FormalPowerSeries quotient;
        if (std::min(quotient_size, b.size()) <= 32) {
            quotient.resize(quotient_size);
            const auto inverse_leading = reinsirk::detail::fps_power(b.coefficients_.back(), modulus() - 2);
            for (std::size_t at = quotient_size; at != 0; --at) {
                const auto index = at - 1;
                const auto coefficient = reinsirk::detail::fps_multiply(a.coefficients_[index + b.size() - 1], inverse_leading);
                quotient.coefficients_[index] = coefficient;
                for (std::size_t j = 0; j < b.size(); ++j)
                    a.coefficients_[index + j] = reinsirk::detail::fps_subtract(a.coefficients_[index + j],
                        reinsirk::detail::fps_multiply(coefficient, b.coefficients_[j]));
            }
            a.resize(b.size() - 1); a.trim(); quotient.trim();
            return {std::move(quotient), std::move(a)};
        }
        reinsirk::detail::fps_require_precision(quotient_size);
        std::vector<std::uint32_t> reversed_a(quotient_size), reversed_b(std::min(quotient_size, b.size()));
        std::copy_n(a.coefficients_.rbegin(), quotient_size, reversed_a.begin());
        std::copy_n(b.coefficients_.rbegin(), reversed_b.size(), reversed_b.begin());
        auto reversed_quotient = reinsirk::detail::fps_engine().convolution_normalized(
            std::move(reversed_a), normalized(std::move(reversed_b)).inverse(quotient_size).coefficients_);
        reversed_quotient.resize(quotient_size); std::reverse(reversed_quotient.begin(), reversed_quotient.end());
        quotient = normalized(std::move(reversed_quotient));
        auto remainder = std::move(a) - b * quotient;
        remainder.resize(b.size() - 1); remainder.trim(); quotient.trim();
        return {std::move(quotient), std::move(remainder)};
    }
    static void clear_cache() noexcept {
        reinsirk::detail::fps_engine().clear_cache();
        std::vector<std::uint32_t>{}.swap(reinsirk::detail::fps_integer_inverses());
    }
};
#endif
