#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#ifndef SIRKIT_POLYNOMIAL_DETAIL_DEFINED
#define SIRKIT_POLYNOMIAL_DETAIL_DEFINED
namespace reinsirk::detail {
inline constexpr std::uint32_t polynomial_modulus=998244353;
inline std::uint32_t polynomial_multiply(std::uint32_t a,std::uint32_t b) noexcept{return static_cast<std::uint32_t>(std::uint64_t{a}*b%polynomial_modulus);}
inline std::uint32_t polynomial_power(std::uint32_t a,std::uint32_t e) noexcept {std::uint32_t r=1;for(;e;e>>=1,a=polynomial_multiply(a,a))if(e&1)r=polynomial_multiply(r,a);return r;}
template<std::integral T>
std::uint32_t polynomial_normalize(T value) noexcept {
    static_assert(sizeof(T)<=sizeof(std::uint64_t));
    if constexpr(std::is_signed_v<T>){const auto r=static_cast<std::int64_t>(value)%polynomial_modulus;return static_cast<std::uint32_t>(r<0?r+polynomial_modulus:r);}
    else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value)%polynomial_modulus);
}
inline std::uint32_t polynomial_horner(const FormalPowerSeries& f,std::uint32_t x) noexcept {
    std::uint32_t value=0;
    for(auto i=f.size();i;--i){value=polynomial_multiply(value,x);const auto sum=value+f.get(i-1);value=sum>=polynomial_modulus?sum-polynomial_modulus:sum;}
    return value;
}
}
#endif
#ifndef SIRKIT_PRODUCT_TREE_DEFINED
#define SIRKIT_PRODUCT_TREE_DEFINED
class ProductTree {
    std::size_t base_=0;
    std::vector<std::uint32_t> points_;
    std::vector<FormalPowerSeries> products_;
    void evaluate_node(const FormalPowerSeries& f,std::size_t node,std::size_t begin,std::size_t width,std::vector<std::uint32_t>& out) const {
        if(begin>=size())return;
        const auto count=std::min(width,size()-begin);
        if(count<=32||f.size()<=32){for(std::size_t i=begin;i<begin+count;++i)out[i]=reinsirk::detail::polynomial_horner(f,points_[i]);return;}
        const auto half=width/2;
        {const auto remainder=f%products_[node*2];evaluate_node(remainder,node*2,begin,half,out);}
        if(begin+half<size()){const auto remainder=f%products_[node*2+1];evaluate_node(remainder,node*2+1,begin+half,half,out);}
    }
    template<std::integral T>
    void build(const std::vector<T>& points) {
        if(points.size()>max_points())throw std::length_error("ProductTree: too many points");
        if(points.empty())return;
        base_=std::bit_ceil(points.size());
        points_.reserve(points.size());for(const auto value:points)points_.push_back(reinsirk::detail::polynomial_normalize(value));
        products_.resize(base_*2);
        for(std::size_t i=0;i<base_;++i){
            if(i<size())products_[base_+i]=FormalPowerSeries{points_[i]?std::int64_t{998244353}-points_[i]:0,1};
            else products_[base_+i]=FormalPowerSeries{1};
        }
        for(std::size_t node=base_-1;node;--node){
            if(products_[node*2+1].size()==1)products_[node]=products_[node*2];
            else products_[node]=products_[node*2]*products_[node*2+1];
        }
    }
public:
    static constexpr std::size_t max_points() noexcept{return FormalPowerSeries::max_precision()-1;}
    ProductTree()=default;
    template<std::integral T>explicit ProductTree(const std::vector<T>& points){build(points);}
    ProductTree(const ProductTree&)=default;
    ProductTree(ProductTree&& other) noexcept:base_(std::exchange(other.base_,0)),points_(std::move(other.points_)),products_(std::move(other.products_)){other.points_.clear();other.products_.clear();}
    ProductTree& operator=(const ProductTree& other){if(this!=&other){ProductTree copy(other);swap(copy);}return *this;}
    ProductTree& operator=(ProductTree&& other) noexcept {if(this!=&other){ProductTree moved(std::move(other));swap(moved);}return *this;}
    void swap(ProductTree& other) noexcept {std::swap(base_,other.base_);points_.swap(other.points_);products_.swap(other.products_);}
    friend void swap(ProductTree& a,ProductTree& b) noexcept {a.swap(b);}
    template<std::integral T>void assign(const std::vector<T>& points){ProductTree next(points);swap(next);}
    std::size_t size()const noexcept{return points_.size();}
    bool empty()const noexcept{return points_.empty();}
    const std::vector<std::uint32_t>& points()const noexcept{return points_;}
    FormalPowerSeries polynomial()const{return empty()?FormalPowerSeries{1}:products_[1];}
    std::vector<std::uint32_t> evaluate(const FormalPowerSeries& f)const {
        if(f.size()>FormalPowerSeries::max_precision())throw std::length_error("ProductTree: polynomial too long");
        std::vector<std::uint32_t> result(size());
        if(empty())return result;
        if(size()>32&&f.size()>32&&f.size()>size()){const auto remainder=f%products_[1];evaluate_node(remainder,1,0,base_,result);}
        else evaluate_node(f,1,0,base_,result);
        return result;
    }
    template<std::integral T>std::vector<std::uint32_t> evaluate(const std::vector<T>& coefficients)const{if(coefficients.size()>FormalPowerSeries::max_precision())throw std::length_error("ProductTree: polynomial too long");return evaluate(FormalPowerSeries(coefficients));}
    template<std::integral T>FormalPowerSeries interpolate(const std::vector<T>& values)const {
#ifndef SKIP_RUNTIME_CHECKS
        if(values.size()!=size())throw std::invalid_argument("ProductTree: values size mismatch");
#endif
        if(empty())return {};
        const auto denominators=evaluate(products_[1].derivative());
        std::vector<std::uint32_t> prefix(size()+1,1);
        for(std::size_t i=0;i<size();++i){
            if(denominators[i]==0)throw std::domain_error("ProductTree: repeated point");
            prefix[i+1]=reinsirk::detail::polynomial_multiply(prefix[i],denominators[i]);
        }
        auto inverse=reinsirk::detail::polynomial_power(prefix.back(),998244351);
        std::vector<FormalPowerSeries> values_tree(base_*2);
        for(std::size_t i=size();i;--i){
            const auto at=i-1;
            const auto weight=reinsirk::detail::polynomial_multiply(reinsirk::detail::polynomial_normalize(values[at]),reinsirk::detail::polynomial_multiply(prefix[at],inverse));
            inverse=reinsirk::detail::polynomial_multiply(inverse,denominators[at]);
            if(weight)values_tree[base_+at]=FormalPowerSeries{weight};
        }
        for(std::size_t node=base_-1;node;--node){
            const auto left=node*2,right=left+1;
            if(!values_tree[left].empty())values_tree[node]=values_tree[left]*products_[right];
            if(!values_tree[right].empty())values_tree[node]+=values_tree[right]*products_[left];
        }
        values_tree[1].resize(size());return std::move(values_tree[1]);
    }
};
#endif
#ifndef SIRKIT_MULTIPOINT_EVALUATE_DEFINED
#define SIRKIT_MULTIPOINT_EVALUATE_DEFINED
template<std::integral T>
std::vector<std::uint32_t> MultipointEvaluate(const FormalPowerSeries& f,const std::vector<T>& points){return ProductTree(points).evaluate(f);}
template<std::integral T,std::integral U>
std::vector<std::uint32_t> MultipointEvaluate(const std::vector<T>& f,const std::vector<U>& points){return ProductTree(points).evaluate(f);}
#endif
#ifndef SIRKIT_POLYNOMIAL_INTERPOLATE_DEFINED
#define SIRKIT_POLYNOMIAL_INTERPOLATE_DEFINED
template<std::integral T,std::integral U>
FormalPowerSeries PolynomialInterpolate(const std::vector<T>& points,const std::vector<U>& values){return ProductTree(points).interpolate(values);}
#endif
#ifndef SIRKIT_TAYLOR_SHIFT_DEFINED
#define SIRKIT_TAYLOR_SHIFT_DEFINED
template<std::integral T>
FormalPowerSeries TaylorShift(const FormalPowerSeries& f,T shift) {
    const auto n=f.size();if(n>FormalPowerSeries::max_precision())throw std::length_error("TaylorShift: polynomial too long");
    if(!n)return {};
    std::vector<std::uint32_t> factorial(n,1),inverse_factorial(n,1),left(n),right(n);
    for(std::size_t i=1;i<n;++i)factorial[i]=reinsirk::detail::polynomial_multiply(factorial[i-1],static_cast<std::uint32_t>(i));
    inverse_factorial[n-1]=reinsirk::detail::polynomial_power(factorial[n-1],998244351);
    for(std::size_t i=n-1;i;--i)inverse_factorial[i-1]=reinsirk::detail::polynomial_multiply(inverse_factorial[i],static_cast<std::uint32_t>(i));
    const auto c=reinsirk::detail::polynomial_normalize(shift);std::uint32_t power=1;
    for(std::size_t i=0;i<n;++i){left[n-1-i]=reinsirk::detail::polynomial_multiply(f.get(i),factorial[i]);right[i]=reinsirk::detail::polynomial_multiply(power,inverse_factorial[i]);power=reinsirk::detail::polynomial_multiply(power,c);}
    const auto product=FormalPowerSeries(std::move(left))*FormalPowerSeries(std::move(right));
    std::vector<std::uint32_t> result(n);
    for(std::size_t i=0;i<n;++i)result[i]=reinsirk::detail::polynomial_multiply(product.get(n-1-i),inverse_factorial[i]);
    return FormalPowerSeries(std::move(result));
}
template<std::integral T,std::integral U>
FormalPowerSeries TaylorShift(const std::vector<T>& f,U shift){if(f.size()>FormalPowerSeries::max_precision())throw std::length_error("TaylorShift: polynomial too long");return TaylorShift(FormalPowerSeries(f),shift);}
#endif
