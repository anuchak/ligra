// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include <chrono>
#include "ligra.h"

struct BFS_F {
  uintE* Parents;
  BFS_F(uintE* _Parents) : Parents(_Parents) {}
  inline bool update (uintE s, uintE d) { //Update
    if(Parents[d] == UINT_E_MAX) { Parents[d] = s; return 1; }
    else return 0;
  }
  inline bool updateAtomic (uintE s, uintE d){ //atomic version of Update
    return (CAS(&Parents[d],UINT_E_MAX,s));
  }
  //cond function checks if vertex has been visited yet
  inline bool cond (uintE d) { return (Parents[d] == UINT_E_MAX); }
};

template <class vertex>
void Compute(graph<vertex>& GA, commandLine P) {
  for (auto round = 0u; round < 2u; round++) {
  auto duration = std::chrono::system_clock::now().time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  // re-using the rounds parameter for
  long start = P.getOptionLongValue("-r",0);
  for (auto source = 0u; source < start; source++) {
   // printf("source: %lu \n", source);
   // auto duration2 = std::chrono::system_clock::now().time_since_epoch();
   // auto millis2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration2).count();
  long n = GA.n;
  //creates Parents array, initialized to all -1, except for start
  uintE* Parents = newA(uintE,n);
  parallel_for(long i=0;i<n;i++) Parents[i] = UINT_E_MAX;
  Parents[source] = source;
  vertexSubset Frontier(n,source); //creates initial frontier
  int level = 0;
  while(!Frontier.isEmpty()) { //loop until frontier is empty
    //auto duration3 = std::chrono::system_clock::now().time_since_epoch();
    //auto millis3 = std::chrono::duration_cast<std::chrono::milliseconds>(duration3).count();
    //printf("size of frontier %lu \n", Frontier.size());
    vertexSubset output = edgeMap(GA, Frontier, BFS_F(Parents));
    Frontier.del();
    Frontier = output; //set new frontier
    /*auto duration2 = std::chrono::system_clock::now().time_since_epoch();
    auto millis2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration2).count();
    printf("time taken for frontier: %lu ms\n", millis2 - millis3);*/
    level++;
    // if some source is not connected at all (no work), then increment total sources given
    // ensure fairness of work, exactly 1 / 8 / 64 sources with work are used for BFS
    if (level == 1 && Frontier.size() == 0) {
      start++;
    }
  }
  Frontier.del();
  free(Parents);
   //auto duration3 = std::chrono::system_clock::now().time_since_epoch();
   //auto millis3 = std::chrono::duration_cast<std::chrono::milliseconds>(duration3).count();
   //printf("total time for source: %lu | %lu ms\n", source, millis3 - millis2);
  }
  auto duration1 = std::chrono::system_clock::now().time_since_epoch();
  auto millis1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration1).count();
  printf("total time: %lu\n", millis1 - millis);
  }
}
