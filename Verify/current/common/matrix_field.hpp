// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <stdexcept>
#ifndef SIRKIT_VERIFY_MATRIX_FIELD_DEFINED
#define SIRKIT_VERIFY_MATRIX_FIELD_DEFINED
struct VerifyMatrixField {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    VerifyMatrixField()=default;
    VerifyMatrixField(std::uint64_t input):value(static_cast<std::uint32_t>(input%modulus)){}
    std::uint32_t get()const noexcept{return value;}
    VerifyMatrixField& operator+=(VerifyMatrixField b)noexcept{value+=b.value;if(value>=modulus)value-=modulus;return *this;}
    VerifyMatrixField& operator-=(VerifyMatrixField b)noexcept{value=value>=b.value?value-b.value:value+modulus-b.value;return *this;}
    VerifyMatrixField& operator*=(VerifyMatrixField b)noexcept{value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;}
    friend VerifyMatrixField operator+(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a+=b;}
    friend VerifyMatrixField operator-(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a-=b;}
    friend VerifyMatrixField operator*(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a*=b;}
    friend bool operator==(VerifyMatrixField,VerifyMatrixField)=default;
    friend VerifyMatrixField operator/(VerifyMatrixField a,VerifyMatrixField b){
        if(b.value==0)throw std::domain_error("verification field: zero divisor");
        VerifyMatrixField inverse{1};std::uint32_t exponent=modulus-2;
        while(exponent){if(exponent&1)inverse*=b;b*=b;exponent>>=1;}return a*inverse;
    }
};
#endif
