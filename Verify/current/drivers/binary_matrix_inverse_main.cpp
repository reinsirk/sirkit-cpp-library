// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>
int main() {
    std::size_t n{};verify_in>>n;
    BinaryMatrix a(n,n);
    for(std::size_t i=0;i<n;++i){std::string row;verify_in>>row;for(std::size_t j=0;j<n;++j)a.set(i,j,row[j]=='1');}
    const auto result=a.inverse();
    if(!result){verify_out<<-1<<'\n';return 0;}
    for(std::size_t i=0;i<n;++i){for(std::size_t j=0;j<n;++j)verify_out<<(result->test(i,j)?'1':'0');verify_out<<'\n';}
}
