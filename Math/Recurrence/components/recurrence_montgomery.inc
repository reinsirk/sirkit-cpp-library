#include <cstdint>
#if defined(__clang__)
#ifndef SIRKIT_RECURRENCE_MONTGOMERY_DEFINED
#define SIRKIT_RECURRENCE_MONTGOMERY_DEFINED
namespace reinsirk::detail {
struct RecurrenceMontgomery998 {
    static constexpr std::uint32_t modulus=998244353;
    static constexpr std::uint32_t negative_inverse=[] {
        std::uint32_t inverse=1;
        for(unsigned i=0;i<5;++i)inverse*=2-modulus*inverse;
        return std::uint32_t{0}-inverse;
    }();
    static constexpr std::uint64_t r=(std::uint64_t{1}<<32)%modulus;
    static constexpr std::uint32_t r_squared=static_cast<std::uint32_t>(r*r%modulus);
    std::uint32_t raw=0;
    static constexpr std::uint32_t reduce(std::uint64_t product)noexcept {
        const auto factor=static_cast<std::uint32_t>(product)*negative_inverse;
        // product<p²かつp<2^30なので、この和はuint64_tの範囲内。
        const auto result=static_cast<std::uint32_t>((product+std::uint64_t{factor}*modulus)>>32);
        return result>=modulus?result-modulus:result;
    }
    RecurrenceMontgomery998()=default;
    RecurrenceMontgomery998(std::uint64_t x):raw(reduce((x%modulus)*r_squared)){}
    std::uint32_t get()const noexcept{return reduce(raw);}
    RecurrenceMontgomery998& operator+=(RecurrenceMontgomery998 b)noexcept {
        raw+=b.raw;if(raw>=modulus)raw-=modulus;return *this;
    }
    RecurrenceMontgomery998& operator-=(RecurrenceMontgomery998 b)noexcept {
        raw=raw>=b.raw?raw-b.raw:raw+modulus-b.raw;return *this;
    }
    RecurrenceMontgomery998& operator*=(RecurrenceMontgomery998 b)noexcept {
        raw=reduce(std::uint64_t{raw}*b.raw);return *this;
    }
    friend RecurrenceMontgomery998 operator+(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept{return a+=b;}
    friend RecurrenceMontgomery998 operator-(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept{return a-=b;}
    friend RecurrenceMontgomery998 operator*(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept{return a*=b;}
    friend bool operator==(RecurrenceMontgomery998,RecurrenceMontgomery998)=default;
    friend RecurrenceMontgomery998 operator/(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept {
        RecurrenceMontgomery998 inverse{1};std::uint32_t power=modulus-2;
        while(power){if(power&1)inverse*=b;b*=b;power>>=1;}return a*inverse;
    }
};
}
#endif
#endif
