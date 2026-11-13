#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_CRT_CONVOLUTION_DETAIL_DEFINED
#define SIRKIT_CRT_CONVOLUTION_DETAIL_DEFINED
namespace reinsirk::detail {
inline constexpr std::uint32_t convolution_prime0=167772161,convolution_prime1=469762049,convolution_prime2=998244353;
inline constexpr std::uint64_t convolution_product01=std::uint64_t{convolution_prime0}*convolution_prime1;
constexpr std::uint32_t convolution_power(std::uint32_t a,std::uint32_t e,std::uint32_t p) noexcept {
    std::uint32_t r=1;
    for(;e;e>>=1,a=static_cast<std::uint32_t>(std::uint64_t{a}*a%p))if(e&1)r=static_cast<std::uint32_t>(std::uint64_t{r}*a%p);
    return r;
}
inline constexpr auto convolution_inverse0=convolution_power(convolution_prime0,convolution_prime1-2,convolution_prime1);
inline constexpr auto convolution_inverse01=convolution_power(static_cast<std::uint32_t>(convolution_product01%convolution_prime2),convolution_prime2-2,convolution_prime2);
struct ConvolutionDigits {std::uint64_t low;std::uint32_t high;};
inline ConvolutionDigits convolution_crt_digits(std::uint32_t a,std::uint32_t b,std::uint32_t c) noexcept {
    const auto delta1=b>=a?b-a:b+convolution_prime1-a;
    const auto digit1=std::uint64_t{delta1}*convolution_inverse0%convolution_prime1;
    const auto low=std::uint64_t{a}+std::uint64_t{convolution_prime0}*digit1;
    const auto residue=static_cast<std::uint32_t>(low%convolution_prime2);
    const auto delta2=c>=residue?c-residue:c+convolution_prime2-residue;
    return {low,static_cast<std::uint32_t>(std::uint64_t{delta2}*convolution_inverse01%convolution_prime2)};
}
struct ConvolutionWide {std::uint64_t low,high;};
constexpr ConvolutionWide convolution_wide_product(std::uint64_t a,std::uint32_t b,std::uint64_t add=0) noexcept {
    const auto lower=(a&0xffffffffu)*b;
    const auto upper=(a>>32)*b;
    const auto low=lower+(upper<<32);
    const auto result=low+add;
    return {result,(upper>>32)+(low<lower)+(result<low)};
}
inline std::int64_t convolution_decode_integer(ConvolutionDigits digits) {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__)) && !defined(SIRKIT_CONVOLUTION_DISABLE_INT128) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
    __extension__ using Wide=unsigned __int128;
    constexpr Wide product=Wide{convolution_product01}*convolution_prime2;
    const Wide value=Wide{convolution_product01}*digits.high+digits.low;
    const bool negative=value>product/2;
    const Wide magnitude=negative?product-value:value;
    const Wide limit=Wide{std::numeric_limits<std::int64_t>::max()}+(negative?1u:0u);
    if(magnitude>limit)throw std::overflow_error("CRTConvolution: coefficient outside int64_t");
    const auto small=static_cast<std::uint64_t>(magnitude);
#else
    constexpr auto product=convolution_wide_product(convolution_product01,convolution_prime2);
    const auto value=convolution_wide_product(convolution_product01,digits.high,digits.low);
    constexpr ConvolutionWide half{(product.low>>1)|(product.high<<63),product.high>>1};
    const bool negative=value.high>half.high||(value.high==half.high&&value.low>half.low);
    const ConvolutionWide magnitude=negative?ConvolutionWide{product.low-value.low,product.high-value.high-(product.low<value.low)}:value;
    const auto limit=std::uint64_t{std::numeric_limits<std::int64_t>::max()}+(negative?1u:0u);
    if(magnitude.high||magnitude.low>limit)throw std::overflow_error("CRTConvolution: coefficient outside int64_t");
    const auto small=magnitude.low;
#endif
    if(!negative)return static_cast<std::int64_t>(small);
    if(small==(std::uint64_t{1}<<63))return std::numeric_limits<std::int64_t>::min();
    return -static_cast<std::int64_t>(small);
}
template<std::integral T>
std::uint32_t convolution_normalize(T value,std::uint32_t mod) noexcept {
    static_assert(sizeof(T)<=sizeof(std::uint64_t));
    if constexpr(std::is_signed_v<T>){const auto r=static_cast<std::int64_t>(value)%static_cast<std::int64_t>(mod);return static_cast<std::uint32_t>(r<0?r+mod:r);}
    else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value)%mod);
}
}
#endif

#ifndef SIRKIT_CRT_CONVOLUTION_DEFINED
#define SIRKIT_CRT_CONVOLUTION_DEFINED
class CRTConvolution {
    NTT<167772161> first_;
    NTT<469762049> second_;
    NTT<998244353> third_;
    static std::size_t output_size(std::size_t a,std::size_t b) {
        if(!a||!b)return 0;
        if(a-1>std::numeric_limits<std::size_t>::max()-b)throw std::length_error("CRTConvolution: size overflow");
        const auto n=a-1+b;
        if(n>max_length())throw std::length_error("CRTConvolution: result too long");
        return n;
    }
    template<bool Square,std::integral T,std::integral U>
    std::vector<std::uint32_t> modular_impl(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod) {
        if(mod==0)throw std::invalid_argument("CRTConvolution: zero modulus");
        const auto n=output_size(a.size(),b.size());
        if(!n)return {};
        if(mod==1)return std::vector<std::uint32_t>(n);
        std::vector<std::uint32_t> x; x.reserve(a.size());
        for(const auto v:a)x.push_back(reinsirk::detail::convolution_normalize(v,mod));
        std::vector<std::uint32_t> y;
        if constexpr(!Square){y.reserve(b.size());for(const auto v:b)y.push_back(reinsirk::detail::convolution_normalize(v,mod));}
        const auto& right=Square?x:y;
        if(std::min(x.size(),right.size())<=32){
            std::vector<std::uint32_t> out(n);
            for(std::size_t i=0;i<x.size();++i)for(std::size_t j=0;j<right.size();++j)
                out[i+j]=static_cast<std::uint32_t>((std::uint64_t{x[i]}*right[j]+out[i+j])%mod);
            return out;
        }
        auto compute=[&](auto& ntt){if constexpr(Square)return ntt.square(x);else return ntt.convolution(x,y);};
        if(mod==167772161)return compute(first_);
        if(mod==469762049)return compute(second_);
        if(mod==998244353)return compute(third_);
        auto r0=compute(first_),r1=compute(second_),r2=compute(third_);
        const auto factor=reinsirk::detail::convolution_product01%mod;
        for(std::size_t i=0;i<n;++i){
            const auto digits=reinsirk::detail::convolution_crt_digits(r0[i],r1[i],r2[i]);
            r0[i]=static_cast<std::uint32_t>((digits.low%mod+factor*(digits.high%mod))%mod);
        }
        return r0;
    }
    template<bool Square>
    std::vector<std::int64_t> integer_impl(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b) {
        const auto n=output_size(a.size(),b.size());
        if(!n)return {};
        auto compute=[&](auto& ntt){if constexpr(Square)return ntt.square(a);else return ntt.convolution(a,b);};
        const auto r0=compute(first_),r1=compute(second_),r2=compute(third_);
        std::vector<std::int64_t> out(n);
        for(std::size_t i=0;i<n;++i)out[i]=reinsirk::detail::convolution_decode_integer(reinsirk::detail::convolution_crt_digits(r0[i],r1[i],r2[i]));
        return out;
    }
public:
    static constexpr std::size_t max_length() noexcept {return std::size_t{1}<<23;}
    void prepare(std::size_t length) {
        if(length>max_length()||(length&&!std::has_single_bit(length)))throw std::length_error("CRTConvolution: invalid transform length");
        first_.prepare(length);second_.prepare(length);third_.prepare(length);
    }
    std::size_t roots_capacity() const noexcept {return first_.roots_capacity()+second_.roots_capacity()+third_.roots_capacity();}
    void clear_cache() noexcept {first_.clear_cache();second_.clear_cache();third_.clear_cache();}
    template<std::integral T,std::integral U>
    std::vector<std::uint32_t> modular(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod){return modular_impl<false>(a,b,mod);}
    template<std::integral T>
    std::vector<std::uint32_t> modular_square(const std::vector<T>& a,std::uint32_t mod){return modular_impl<true>(a,a,mod);}
    std::vector<std::int64_t> integer(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b){return integer_impl<false>(a,b);}
    std::vector<std::int64_t> integer_square(const std::vector<std::int64_t>& a){return integer_impl<true>(a,a);}
};
#endif

#ifndef SIRKIT_CONVOLUTION_ANY_MOD_DEFINED
#define SIRKIT_CONVOLUTION_ANY_MOD_DEFINED
template<std::integral T,std::integral U>
std::vector<std::uint32_t> ConvolutionAnyMod(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod){return CRTConvolution{}.modular(a,b,mod);}
#endif
#ifndef SIRKIT_CONVOLUTION_INT64_DEFINED
#define SIRKIT_CONVOLUTION_INT64_DEFINED
inline std::vector<std::int64_t> ConvolutionInt64(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b){return CRTConvolution{}.integer(a,b);}
#endif
