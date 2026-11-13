#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#ifndef SIRKIT_LINEAR_RECURRENCE_DETAIL_DEFINED
#define SIRKIT_LINEAR_RECURRENCE_DETAIL_DEFINED
namespace reinsirk::detail {
template<class T>
std::size_t recurrence_product_size(std::size_t d) {
    const auto maximum=std::vector<T>{}.max_size();
    if(d>maximum/2+maximum%2)throw std::length_error("LinearRecurrence: size overflow");
    return d?d*2-1:0;
}
template<class T>
void recurrence_product(const std::vector<T>& left,const std::vector<T>& right,
                        const std::vector<T>& coefficients,std::vector<T>& workspace) {
    const auto d=coefficients.size();
    workspace.assign(recurrence_product_size<T>(d),T{0});
    for(std::size_t i=0;i<d;++i)
        for(std::size_t j=0;j<d;++j)workspace[i+j]+=left[i]*right[j];
    for(std::size_t i=workspace.size();i-->d;) {
        const T highest=workspace[i];
        for(std::size_t j=0;j<d;++j)workspace[i-j-1]+=highest*coefficients[j];
    }
    workspace.resize(d);
}
inline std::vector<RecurrenceMod998> recurrence_field_vector(const std::vector<std::uint32_t>& input) {
    std::vector<RecurrenceMod998> result;result.reserve(input.size());
    for(auto x:input)result.emplace_back(x);
    return result;
}
inline std::vector<std::uint32_t> recurrence_residues(const std::vector<RecurrenceMod998>& input) {
    std::vector<std::uint32_t> result;result.reserve(input.size());
    for(auto x:input)result.push_back(x.value);
    return result;
}
}
#endif
#ifndef SIRKIT_LINEAR_RECURRENCE_DEFINED
#define SIRKIT_LINEAR_RECURRENCE_DEFINED
template<class T>
std::vector<T> LinearRecurrenceWeights(const std::vector<T>& coefficients,std::uint64_t k) {
    static_assert(!std::is_integral_v<T>,"LinearRecurrenceWeights<T>: use a coefficient type, or the uint32_t overload");
    const auto d=coefficients.size();
    if(d==0)return {};
    const auto capacity=reinsirk::detail::recurrence_product_size<T>(d);
    if(k<d) {
        std::vector<T> result(d,T{0});result[static_cast<std::size_t>(k)]=T{1};return result;
    }
    std::vector<T> result(d,T{0}),power(d,T{0}),workspace;
    result.reserve(capacity);power.reserve(capacity);workspace.reserve(capacity);
    result[0]=T{1};
    if(d==1)power[0]=coefficients[0];else power[1]=T{1};
    while(k) {
        if(k&1){reinsirk::detail::recurrence_product(result,power,coefficients,workspace);result.swap(workspace);}
        k>>=1;
        if(k){reinsirk::detail::recurrence_product(power,power,coefficients,workspace);power.swap(workspace);}
    }
    return result;
}
template<class T>
T LinearRecurrenceTerm(const std::vector<T>& initial,const std::vector<T>& coefficients,std::uint64_t k) {
    static_assert(!std::is_integral_v<T>,"LinearRecurrenceTerm<T>: use a coefficient type, or the uint32_t overload");
#ifndef SKIP_RUNTIME_CHECKS
    if(initial.size()!=coefficients.size())throw std::invalid_argument("LinearRecurrenceTerm: mismatched sizes");
#endif
    if(k<initial.size())return initial[static_cast<std::size_t>(k)];
    const auto weights=LinearRecurrenceWeights(coefficients,k);
    T result{0};
    for(std::size_t j=0;j<weights.size();++j)result+=initial[j]*weights[j];
    return result;
}
template<class T>
std::vector<T> LinearRecurrencePrefix(const std::vector<T>& initial,const std::vector<T>& coefficients,std::size_t count) {
    static_assert(!std::is_integral_v<T>,"LinearRecurrencePrefix<T>: use a coefficient type, or the uint32_t overload");
#ifndef SKIP_RUNTIME_CHECKS
    if(initial.size()!=coefficients.size())throw std::invalid_argument("LinearRecurrencePrefix: mismatched sizes");
#endif
    std::vector<T> result(count,T{0});
    const auto d=coefficients.size();
    std::copy_n(initial.begin(),std::min(d,count),result.begin());
    for(std::size_t i=d;i<count;++i)
        for(std::size_t j=0;j<d;++j)result[i]+=coefficients[j]*result[i-j-1];
    return result;
}
inline std::vector<std::uint32_t> LinearRecurrenceWeights(const std::vector<std::uint32_t>& coefficients,std::uint64_t k) {
    if(k<coefficients.size()) {
        reinsirk::detail::recurrence_product_size<reinsirk::detail::RecurrenceMod998>(coefficients.size());
        std::vector<std::uint32_t> result(coefficients.size(),0);
        result[static_cast<std::size_t>(k)]=1;return result;
    }
#if defined(__clang__)
    if(coefficients.size()>=16) {
        using Field=reinsirk::detail::RecurrenceMontgomery998;
        std::vector<Field> normalized;normalized.reserve(coefficients.size());
        for(auto value:coefficients)normalized.emplace_back(value);
        const auto weights=LinearRecurrenceWeights(normalized,k);
        std::vector<std::uint32_t> result;result.reserve(weights.size());
        for(auto value:weights)result.push_back(value.get());
        return result;
    }
#endif
    return reinsirk::detail::recurrence_residues(LinearRecurrenceWeights(reinsirk::detail::recurrence_field_vector(coefficients),k));
}
inline std::vector<std::uint32_t> LinearRecurrencePrefix(const std::vector<std::uint32_t>& initial,
                                                      const std::vector<std::uint32_t>& coefficients,std::size_t count) {
    return reinsirk::detail::recurrence_residues(LinearRecurrencePrefix(reinsirk::detail::recurrence_field_vector(initial),reinsirk::detail::recurrence_field_vector(coefficients),count));
}
inline std::uint32_t LinearRecurrenceTerm(const std::vector<std::uint32_t>& initial,
                                        const std::vector<std::uint32_t>& coefficients,std::uint64_t k) {
#ifndef SKIP_RUNTIME_CHECKS
    if(initial.size()!=coefficients.size())throw std::invalid_argument("LinearRecurrenceTerm: mismatched sizes");
#endif
    constexpr std::uint32_t mod=998244353;
    if(k<initial.size())return initial[static_cast<std::size_t>(k)]%mod;
    const auto d=coefficients.size();
    if(d==0)return 0;
    if(d>=std::vector<std::uint32_t>{}.max_size())throw std::length_error("LinearRecurrenceTerm: size overflow");
    std::vector<std::uint32_t> denominator(d+1);denominator[0]=1;
    for(std::size_t j=0;j<d;++j){const auto x=coefficients[j]%mod;denominator[j+1]=x?mod-x:0;}
    while(denominator.size()>1&&denominator.back()==0)denominator.pop_back();
    NTT<> ntt;
    auto numerator=ntt.convolution(initial,denominator);numerator.resize(d);
    while(!numerator.empty()&&numerator.back()==0)numerator.pop_back();
    while(k&&!numerator.empty()) {
        auto reflected=denominator;
        for(std::size_t j=1;j<reflected.size();j+=2)if(reflected[j])reflected[j]=mod-reflected[j];
        auto product=ntt.convolution_normalized(std::move(numerator),reflected);
        auto norm=ntt.convolution_normalized(denominator,std::move(reflected));
        numerator.clear();numerator.reserve((product.size()+1)/2);
        for(std::size_t j=static_cast<std::size_t>(k&1);j<product.size();j+=2)numerator.push_back(product[j]);
        denominator.clear();denominator.reserve((norm.size()+1)/2);
        for(std::size_t j=0;j<norm.size();j+=2)denominator.push_back(norm[j]);
        while(!numerator.empty()&&numerator.back()==0)numerator.pop_back();
        while(denominator.size()>1&&denominator.back()==0)denominator.pop_back();
        k>>=1;
    }
    // 定数項1の分母から始め、Q(x)Q(-x)の偶数次項だけを取るので定数項は常に1。
    return numerator.empty()?0:numerator[0];
}
#endif
