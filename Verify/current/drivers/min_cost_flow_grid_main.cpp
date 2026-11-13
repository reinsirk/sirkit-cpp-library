// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <string>
#include <vector>
int main(){
 int n{},k{};verify_in>>n>>k;const int source=2*n,sink=source+1;MinCostFlow<int,std::int64_t> network(sink+1);
 for(int i=0;i<n;++i){network.add_edge(source,i,k,0);network.add_edge(n+i,sink,k,0);}
 std::vector<std::vector<std::size_t>> id(n,std::vector<std::size_t>(n));for(int i=0;i<n;++i)for(int j=0;j<n;++j){std::int64_t value{};verify_in>>value;id[i][j]=network.add_edge(i,n+j,1,-value);}
 network.add_edge(source,sink,n*k,0);const auto result=network.flow(source,sink,n*k);verify_out<<-result.second<<'\n';
 for(int i=0;i<n;++i){std::string row(n,'.');for(int j=0;j<n;++j)if(network.get_edge(id[i][j]).flow)row[j]='X';verify_out<<row<<'\n';}
}
