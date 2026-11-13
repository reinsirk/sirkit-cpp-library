#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#ifndef SIRKIT_GAUSSIAN_RESULT_DEFINED
#define SIRKIT_GAUSSIAN_RESULT_DEFINED
template<class T>struct GaussianEliminationResult {
    Matrix<T> matrix;
    std::vector<std::size_t> pivot_columns;
    std::vector<T> pivots;
    std::size_t row_swaps=0;
    std::size_t rank()const noexcept{return pivot_columns.size();}
};
template<class T>struct LinearSystemResult {
    std::vector<T> particular;
    Matrix<T> kernel;
    std::vector<std::size_t> pivot_columns;
};
#endif
#ifndef SIRKIT_GAUSSIAN_DETAIL_DEFINED
#define SIRKIT_GAUSSIAN_DETAIL_DEFINED
namespace reinsirk::detail {
template<class T>
GaussianEliminationResult<T> field_eliminate(Matrix<T> matrix,std::size_t pivot_end,bool reduced) {
    static_assert(!std::is_integral_v<T>,"Gaussian elimination requires an exact field type, not an integer type");
    GaussianEliminationResult<T> result{std::move(matrix),{},{},0};
    auto& work=result.matrix;
    result.pivot_columns.reserve(std::min(work.rows(),pivot_end));
    result.pivots.reserve(std::min(work.rows(),pivot_end));
    for(std::size_t column=0;column<pivot_end&&result.rank()<work.rows();++column) {
        const auto rank=result.rank();std::size_t candidate=rank;
        while(candidate<work.rows()&&work.row(candidate)[column]==T{0})++candidate;
        if(candidate==work.rows())continue;
        auto pivot_row=work.row(rank);
        if(candidate!=rank) {
            auto other=work.row(candidate);
            for(std::size_t j=column;j<work.cols();++j)std::swap(pivot_row[j],other[j]);
            ++result.row_swaps;
        }
        const T pivot=pivot_row[column];
        result.pivot_columns.push_back(column);result.pivots.push_back(pivot);
        if(pivot!=T{1}) {
            const T inverse=T{1}/pivot;
            if(pivot*inverse!=T{1})throw std::domain_error("Gaussian elimination: non-invertible pivot");
            for(std::size_t j=column+1;j<work.cols();++j)pivot_row[j]*=inverse;
            pivot_row[column]=T{1};
        }
        const auto begin=reduced?std::size_t{0}:rank+1;
        for(std::size_t i=begin;i<work.rows();++i) {
            if(i==rank)continue;
            auto destination=work.row(i);const T factor=destination[column];
            if(factor==T{0})continue;
            for(std::size_t j=column+1;j<work.cols();++j)destination[j]-=factor*pivot_row[j];
            destination[column]=T{0};
        }
    }
    return result;
}
}
#endif
#ifndef SIRKIT_FIELD_GAUSSIAN_DEFINED
#define SIRKIT_FIELD_GAUSSIAN_DEFINED
template<class T>GaussianEliminationResult<T> GaussianElimination(Matrix<T> matrix,bool reduced=false) {
    const auto columns=matrix.cols();
    return reinsirk::detail::field_eliminate(std::move(matrix),columns,reduced);
}
template<class T>std::size_t MatrixRank(const Matrix<T>& matrix) {
    return GaussianElimination(matrix).rank();
}
template<class T>T MatrixDeterminant(const Matrix<T>& matrix) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.rows()!=matrix.cols())throw std::invalid_argument("MatrixDeterminant: non-square");
#endif
    auto result=GaussianElimination(matrix);
    if(result.rank()!=matrix.rows())return T{0};
    T determinant{1};for(const auto& pivot:result.pivots)determinant*=pivot;
    return (result.row_swaps&1)?T{0}-determinant:determinant;
}
template<class T>std::optional<Matrix<T>> MatrixInverse(const Matrix<T>& matrix) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.rows()!=matrix.cols())throw std::invalid_argument("MatrixInverse: non-square");
#endif
    const auto n=matrix.rows();
    if(n>std::numeric_limits<std::size_t>::max()/2)throw std::length_error("MatrixInverse: dimensions overflow");
    Matrix<T> augmented(n,n*2);
    for(std::size_t i=0;i<n;++i){auto row=augmented.row(i);const auto source=matrix.row(i);std::copy(source.begin(),source.end(),row.begin());row[n+i]=T{1};}
    auto elimination=reinsirk::detail::field_eliminate(std::move(augmented),n,true);
    if(elimination.rank()!=n)return std::nullopt;
    Matrix<T> inverse(n,n);
    for(std::size_t i=0;i<n;++i){const auto row=elimination.matrix.row(i);std::copy(row.begin()+static_cast<std::ptrdiff_t>(n),row.end(),inverse.row(i).begin());}
    return inverse;
}
template<class T>
std::optional<LinearSystemResult<T>> SolveLinearSystem(const Matrix<T>& matrix,const std::vector<T>& rhs) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.rows()!=rhs.size())throw std::invalid_argument("SolveLinearSystem: rhs size mismatch");
#endif
    const auto m=matrix.rows(),n=matrix.cols();
    if(n==std::numeric_limits<std::size_t>::max())throw std::length_error("SolveLinearSystem: dimensions overflow");
    Matrix<T> augmented(m,n+1);
    for(std::size_t i=0;i<m;++i){auto row=augmented.row(i);const auto source=matrix.row(i);std::copy(source.begin(),source.end(),row.begin());row[n]=rhs[i];}
    auto elimination=reinsirk::detail::field_eliminate(std::move(augmented),n,true);
    const auto rank=elimination.rank();
    for(std::size_t i=rank;i<m;++i)if(elimination.matrix.row(i)[n]!=T{0})return std::nullopt;
    LinearSystemResult<T> result{std::vector<T>(n,T{0}),Matrix<T>(n-rank,n),std::move(elimination.pivot_columns)};
    std::vector<std::uint8_t> pivot(n,0);
    for(std::size_t i=0;i<rank;++i){const auto column=result.pivot_columns[i];pivot[column]=1;result.particular[column]=elimination.matrix.row(i)[n];}
    std::size_t basis=0;
    for(std::size_t column=0;column<n;++column) {
        if(pivot[column])continue;
        auto vector=result.kernel.row(basis++);vector[column]=T{1};
        for(std::size_t i=0;i<rank;++i)vector[result.pivot_columns[i]]=T{0}-elimination.matrix.row(i)[column];
    }
    return result;
}
#endif
