#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#ifndef SIRKIT_BERLEKAMP_MASSEY_DEFINED
#define SIRKIT_BERLEKAMP_MASSEY_DEFINED
namespace reinsirk::detail {
inline std::size_t recurrence_size_plus_one(std::size_t size, std::size_t capacity,
                                            const char* error) {
    if (size >= capacity) throw std::length_error(error);
    return size + 1;
}
}
template<class T>
std::vector<T> BerlekampMassey(const std::vector<T>& sequence) {
    static_assert(!std::is_integral_v<T>,"BerlekampMassey<T>: use an exact field type, or the uint32_t overload");
    if(sequence.empty())return {};
    const auto n=sequence.size();
    (void)reinsirk::detail::recurrence_size_plus_one(n, sequence.max_size(), "BerlekampMassey: size overflow");
    std::vector<T> connection{T{1}},previous{T{1}};
    std::size_t order=0,shift=1;
    T previous_inverse{1};
    for(std::size_t position=0;position<n;++position) {
        T discrepancy=sequence[position];
        for(std::size_t j=1;j<=order;++j)discrepancy+=connection[j]*sequence[position-j];
        if(discrepancy==T{0}){++shift;continue;}
        const T ratio=discrepancy*previous_inverse;
        const bool extend=order<=position/2;
        std::vector<T> saved;
        if(extend)saved=connection;
        // 最後の更新位置pで保存したpreviousはp+1項以下。shiftを足してもn+1項以下。
        if(connection.size()<previous.size()+shift)connection.resize(previous.size()+shift,T{0});
        for(std::size_t j=0;j<previous.size();++j)connection[j+shift]-=ratio*previous[j];
        if(extend) {
            order=position+1-order;
            previous=std::move(saved);
            previous_inverse=T{1}/discrepancy;
            shift=1;
        } else ++shift;
    }
    std::vector<T> result(order,T{0});
    for(std::size_t j=0;j<order;++j)result[j]=T{0}-connection[j+1];
    return result;
}
inline std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t>& sequence) {
    using Field=reinsirk::detail::RecurrenceMod998;
    std::vector<Field> input;input.reserve(sequence.size());
    for(const auto value:sequence)input.emplace_back(value);
    const auto coefficients=BerlekampMassey(input);
    std::vector<std::uint32_t> result;result.reserve(coefficients.size());
    for(const auto value:coefficients)result.push_back(value.value);
    return result;
}
#endif
