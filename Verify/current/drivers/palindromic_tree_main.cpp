// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <string>
#include <vector>
int main(){std::string text;verify_in>>text;PalindromicTree<>tree;tree.reserve(text.size());std::vector<std::int32_t>suffixes;suffixes.reserve(text.size());for(unsigned char byte:text)suffixes.push_back(tree.append(byte));verify_out<<tree.distinct_palindrome_count()<<'\n';for(std::int32_t id=1;id<=static_cast<std::int32_t>(tree.distinct_palindrome_count());++id)verify_out<<tree.parent(id)<<' '<<tree.suffix_link(id)<<'\n';for(std::size_t i=0;i<suffixes.size();++i){if(i)verify_out<<' ';verify_out<<suffixes[i];}verify_out<<'\n';}
