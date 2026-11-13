// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
int main() {
    std::size_t n{},m{};verify_in>>n>>m;
    BinaryMatrix a(n,m);
    for(std::size_t i=0;i<n;++i){std::string row;verify_in>>row;for(std::size_t j=0;j<m;++j)a.set(i,j,row[j]=='1');}
    std::string text;verify_in>>text;
    std::vector<std::uint8_t>rhs(n);for(std::size_t i=0;i<n;++i)rhs[i]=text[i]=='1';
    const auto result=a.solve(rhs);
    if(!result){verify_out<<-1<<'\n';return 0;}
    verify_out<<result->kernel.rows()<<'\n';
    for(const auto bit:result->particular){verify_out<<(bit?'1':'0');}verify_out<<'\n';
    for(std::size_t i=0;i<result->kernel.rows();++i){for(std::size_t j=0;j<m;++j)verify_out<<(result->kernel.test(i,j)?'1':'0');verify_out<<'\n';}
}
