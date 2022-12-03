// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "fatsim.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stack>
#include <vector>

std::vector<long> fat_check(const std::vector<long> & fat)
{
  std::vector<std::vector<long>> adj_list;
  std::vector<long> result;
  adj_list.resize(fat.size());
  std::vector<long> endpoints;
  // Tracks node search and length of current search chain

  /**
   * Build adjacency list
   */
  for (unsigned long i = 0; i < fat.size(); i++) {
    if (fat[i] != -1) {
      adj_list[fat[i]].push_back(i); // adds node to adjacency list
    } else if (fat[i] == -1) {
      endpoints.push_back(i); // adds to end_points list if the node points to -1
    }
  }

  std::stack<std::pair<long, long>> stack;
  long currentChain, longestChain, currentNode, longestNode;
  longestChain = 0;
  longestNode = 0;

  for (long endpoint : endpoints) {

    stack.emplace(endpoint, 1);

    // While stack is not empty, perform DFS on each point and record longest chain
    while (stack.size() > 1) {
      currentNode = stack.top().first;
      currentChain = stack.top().second;
      printf("current node is: %ld\n\n", currentNode);
      printf("current chain is: %ld\n\n", currentChain);
      std::cout << std::endl;
      stack.pop();

      if (currentChain >= longestChain) {
        longestChain = currentChain;
        longestNode = currentNode;
      }

      for (auto n : adj_list[currentNode]) { stack.emplace(n, currentChain + 1); }
    }
    std::cout << "longChain: " << longestChain << std::endl << std::endl << std::endl;
    if (longestChain > 0) { result.push_back(longestNode); }
  }

  std::sort(result.begin(), result.end());
  return result;
}
