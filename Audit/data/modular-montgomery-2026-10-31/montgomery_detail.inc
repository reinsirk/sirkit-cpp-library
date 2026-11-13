#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__)) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
#ifndef SIRKIT_MODULAR_MONTGOMERY64_DEFINED
#define SIRKIT_MODULAR_MONTGOMERY64_DEFINED
// 奇数法・正規化済みの2項。全uint64_t法で、戻り値も常に法未満。
template<bool Subtractive>
class ModularMontgomery64 {
    using Wide=unsigned __int128;
    std::uint64_t modulus_,inverse_,r_squared_;
public:
    explicit constexpr ModularMontgomery64(std::uint64_t modulus) noexcept
        :modulus_(modulus),inverse_(modulus),r_squared_(static_cast<std::uint64_t>((Wide{0}-modulus)%modulus)) {
        // 奇数の二乗はmod8で1。5回のNewton反復で64bit以上の逆元を得る。
        for(unsigned i=0;i<5;++i)inverse_*=2u-modulus_*inverse_;
    }
    constexpr std::uint64_t multiply(std::uint64_t a,std::uint64_t b)const noexcept {
        const Wide product=Wide{a}*b;
        if constexpr(Subtractive){
            const auto high=static_cast<std::uint64_t>(product>>64);
            const auto correction=static_cast<std::uint64_t>((Wide{static_cast<std::uint64_t>(product)*inverse_}*modulus_)>>64);
            const auto value=high-correction;
            return high<correction?value+modulus_:value;
        }else{
            const auto factor=static_cast<std::uint64_t>(product)*(std::uint64_t{0}-inverse_);
            const Wide sum=product+Wide{factor}*modulus_;
            const bool carry=sum<product;
            const auto high=static_cast<std::uint64_t>(sum>>64);
            // 本来の和は129bitまで。carry時は(R+high)-modulusをuint64_tの折返しで表す。
            return carry||high>=modulus_?high-modulus_:high;
        }
    }
    constexpr std::uint64_t encode(std::uint64_t residue)const noexcept{return multiply(residue,r_squared_);}
    constexpr std::uint64_t decode(std::uint64_t residue)const noexcept{return multiply(residue,1);}
    template<bool LeftToRight=true>
    constexpr std::uint64_t power(std::uint64_t residue,std::uint64_t exponent)const noexcept {
        if(!exponent)return 1%modulus_;
        auto base=encode(residue);
        if constexpr(LeftToRight){
            auto value=base;
            for(auto bit=std::bit_floor(exponent)>>1;bit;bit>>=1){value=multiply(value,value);if(exponent&bit)value=multiply(value,base);}
            return decode(value);
        }else{
            auto value=encode(1);
            while(exponent){if(exponent&1)value=multiply(value,base);exponent>>=1;if(exponent)base=multiply(base,base);}
            return decode(value);
        }
    }
};
#endif
#endif
