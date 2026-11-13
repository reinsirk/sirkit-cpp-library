// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main(){
 std::string prefix,format;int n{},m{};verify_in>>prefix>>format>>n>>m;two_sat constraints(n);constraints.reserve_clauses(m);
 for(int i=0;i<m;++i){int a{},b{},zero{};verify_in>>a>>b>>zero;constraints.add_clause((a>0?a:-a)-1,a>0,(b>0?b:-b)-1,b>0);}
 if(!constraints.satisfiable()){verify_out<<"s UNSATISFIABLE\n";return 0;}
 verify_out<<"s SATISFIABLE\nv";for(int i=0;i<n;++i)verify_out<<' '<<(constraints.value(i)?i+1:-i-1);verify_out<<" 0\n";
}
