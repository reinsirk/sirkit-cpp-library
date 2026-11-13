// SPDX-License-Identifier: CC0-1.0
#include <iostream>
#include "../DataStructure/Slope_Trick.cpp"
int main(){
 SlopeTrick f;f.add_abs(2);f.add_abs(8);f.add_constant(3);
 auto interval=f.argmin();std::cout<<*interval.lower<<' '<<*interval.upper<<' '<<f.min_value()<<'\n'; // 2 8 9
 f.shift(-1,2);interval=f.argmin();std::cout<<*interval.lower<<' '<<*interval.upper<<'\n'; // 1 10
 std::cout<<f.eval(0)<<'\n'; // 11
 auto g=f;g.clear_right();std::cout<<g.eval(100)<<'\n'; // 9
 SlopeTrick h;h.add_abs(5);f.merge(h);std::cout<<f.min_value()<<' '<<f.eval(5)<<'\n'; // 9 9
 f.clear();std::cout<<f.empty()<<' '<<f.min_value()<<'\n'; // 1 0
}
