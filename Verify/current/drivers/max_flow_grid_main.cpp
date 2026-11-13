// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
int main(){
 int height{},width{};verify_in>>height>>width;std::vector<std::string> board(height);for(auto& row:board)verify_in>>row;
 const int source=height*width,sink=source+1;mf_graph<int> network(sink+1);
 const int di[4]={1,0,-1,0},dj[4]={0,1,0,-1};
 for(int i=0;i<height;++i)for(int j=0;j<width;++j){
  if(board[i][j]!='.')continue;const int v=i*width+j;
  if((i+j)&1){network.add_edge(v,sink,1);continue;}
  network.add_edge(source,v,1);
  for(int direction=0;direction<4;++direction){int ni=i+di[direction],nj=j+dj[direction];if(ni>=0&&ni<height&&nj>=0&&nj<width&&board[ni][nj]=='.')network.add_edge(v,ni*width+nj,1);}
 }
 verify_out<<network.flow(source,sink)<<'\n';
 for(const auto edge:network.edges()){
  if(edge.from==source||edge.to==sink||edge.flow==0)continue;
  int a=edge.from,b=edge.to;if(a>b)std::swap(a,b);const int ai=a/width,aj=a%width,bi=b/width,bj=b%width;
  if(ai==bi){board[ai][aj]='>';board[bi][bj]='<';}else{board[ai][aj]='v';board[bi][bj]='^';}
 }
 for(const auto& row:board)verify_out<<row<<'\n';
}
