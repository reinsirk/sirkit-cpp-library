// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/implicit_treap.cpp"
#endif
#include HEADER
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
struct Value {
 std::string text;
 static inline bool fail_copy=false;
 explicit Value(std::string s):text(std::move(s)){}
 Value(const Value& x):text(x.text){if(fail_copy)throw std::runtime_error("copy injected");}
 Value& operator=(const Value& x){if(fail_copy)throw std::runtime_error("copy injected");text=x.text;return *this;}
 Value(Value&&)noexcept=default;Value&operator=(Value&&)noexcept=default;
};
Value concatenate(Value a,Value b){a.text+=b.text;return a;}
Value empty_value(){return Value("");}
Value map_value(int,Value x){return x;}
int compose_value(int,int){return 0;}int identity_value(){return 0;}
using Tree=indexed_implicit_treap<Value,concatenate,empty_value,int,map_value,compose_value,identity_value>;
int main(){static_assert(!std::is_nothrow_copy_constructible_v<Value>);int cases=0;
 for(int n=1;n<=32;++n){Tree base;std::vector<std::string>reference;
  for(int i=0;i<n;++i){std::string s(1,char('a'+i%26));base.insert_at_with_id(i,i,Value(s));reference.push_back(s);}
  for(int l=0;l<n;++l)for(int r=l+1;r<=n;++r){Tree t=base;Value::fail_copy=true;bool failed=false;try{(void)t.erase_prod(l,r);}catch(const std::runtime_error&){failed=true;}Value::fail_copy=false;assert(failed);assert(t.size()==n);auto v=t.to_vector();for(int i=0;i<n;++i){assert(v[i].text==reference[i]);assert(t.position_of(i)==i);assert(t.get_id(i)==i);}++cases;}
 }
 std::cout<<cases<<" failed aggregate copies preserved sequence and IDs\n";
}
