// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/implicit_treap.cpp"
#endif
#include HEADER
#include <cassert>
#include <iostream>
#include <stdexcept>
struct Value {
 std::int64_t sum,count;
 static inline bool fail_move=false;
 Value(std::int64_t x,std::int64_t n):sum(x),count(n){}
 Value(const Value&)noexcept=default;Value& operator=(const Value&)noexcept=default;
 Value(Value&& x):sum(x.sum),count(x.count){if(fail_move)throw std::runtime_error("move injected");}
 Value& operator=(Value&&)noexcept=default;
};
Value add(Value a,Value b){a.sum+=b.sum;a.count+=b.count;return a;}
Value empty_value(){return{0,0};}
Value mapping_value(int f,Value x){x.sum+=f*x.count;return x;}
int compose_value(int a,int b){return a+b;}int identity_value(){return 0;}
using Tree=indexed_implicit_treap<Value,add,empty_value,int,mapping_value,compose_value,identity_value,true>;
int main(){static_assert(std::is_nothrow_copy_constructible_v<Value>);static_assert(!std::is_nothrow_move_constructible_v<Value>);Tree t;t.insert_at_with_id(0,3,{42,1});Value::fail_move=true;bool failed=false;try{(void)t.erase_prod(0,1);}catch(const std::runtime_error&){failed=true;}Value::fail_move=false;assert(failed);assert(t.size()==1);assert(t.position_of(3)==0);assert(t.get_by_id(3).sum==42);std::cout<<"throwing move preserved sequence and ID\n";}
