// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>
int main() {
    std::size_t n{}, m{}, k{}; verify_in >> n >> m >> k;
    BinaryMatrix a(n,m), b(m,k);
    for(std::size_t i=0;i<n;++i){std::string row;verify_in>>row;for(std::size_t j=0;j<m;++j)a.set(i,j,row[j]=='1');}
    for(std::size_t i=0;i<m;++i){std::string row;verify_in>>row;for(std::size_t j=0;j<k;++j)b.set(i,j,row[j]=='1');}
    const auto c=a*b;
    for(std::size_t i=0;i<n;++i){for(std::size_t j=0;j<k;++j)verify_out<<(c.test(i,j)?'1':'0');verify_out<<'\n';}
}
