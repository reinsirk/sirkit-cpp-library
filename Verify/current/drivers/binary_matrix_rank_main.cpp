// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>
int main() {
    std::size_t n{},m{};verify_in>>n>>m;
    BinaryMatrix a(n,m);
    if(m)for(std::size_t i=0;i<n;++i){std::string row;verify_in>>row;for(std::size_t j=0;j<m;++j)a.set(i,j,row[j]=='1');}
    verify_out<<a.rank()<<'\n';
}
