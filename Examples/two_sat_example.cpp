// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Two_SAT.cpp"
#include <cassert>
#include <iostream>
int main(){
 two_sat constraints(3);
 constraints.add_clause(0,true,1,true);constraints.imply(1,true,2,false);constraints.add_xor(0,2);constraints.force(0,true);
 assert(constraints.satisfiable());assert(constraints.value(0)&&!constraints.value(2));
 const auto answer=constraints.answer();
 for(bool value:answer)std::cout<<int(value)<<' ';std::cout<<'\n';
 constraints.force(0,false);assert(!constraints.satisfiable());
}
