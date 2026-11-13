// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
int main(){
 int q{};verify_in>>q;SlopeTrick function;
 while(q--){int type{};verify_in>>type;if(type==1){std::int64_t a{},b{};verify_in>>a>>b;function.add_abs(a);function.add_constant(b);}else verify_out<<*function.argmin().lower<<' '<<function.min_value()<<'\n';}
}
