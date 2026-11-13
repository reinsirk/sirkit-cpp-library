#include <cstdint>
#ifndef SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
#define SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
namespace reinsirk::detail {
struct RecurrenceMod998 {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    RecurrenceMod998()=default;
    RecurrenceMod998(std::uint64_t x):value(static_cast<std::uint32_t>(x%modulus)){}
    RecurrenceMod998& operator+=(RecurrenceMod998 b)noexcept{
        value+=b.value;if(value>=modulus)value-=modulus;return *this;
    }
    RecurrenceMod998& operator-=(RecurrenceMod998 b)noexcept{
        value=value>=b.value?value-b.value:value+modulus-b.value;return *this;
    }
    RecurrenceMod998& operator*=(RecurrenceMod998 b)noexcept{
        value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;
    }
    friend RecurrenceMod998 operator+(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a+=b;}
    friend RecurrenceMod998 operator-(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a-=b;}
    friend RecurrenceMod998 operator*(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a*=b;}
    friend bool operator==(RecurrenceMod998,RecurrenceMod998)=default;
    friend RecurrenceMod998 operator/(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{
        RecurrenceMod998 inverse{1};std::uint32_t power=modulus-2;
        while(power){if(power&1)inverse*=b;b*=b;power>>=1;}return a*inverse;
    }
};
}
#endif
