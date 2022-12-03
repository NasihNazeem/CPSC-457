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

long DFS(std::vector<std::vector<long>> & adj_list, long start)
{
  long maxDepth = 1;
  std::vector<std::pair<long, long>> stack;
  stack.push_back(std::make_pair(start, 1));
  while (! stack.empty()) {
    long node = stack.back().first;
    long depth = stack.back().second;
    stack.pop_back();
    if (adj_list.at(node).empty() && maxDepth < depth) {
      maxDepth = depth;
      continue;
    }
    for (auto & elem : adj_list.at(node)) { stack.push_back(std::make_pair(elem, depth + 1)); }
  }
  return maxDepth;
}

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

  long counter = 0;

  for (auto & elem : fat) {
    if (elem == -1) { result.push_back(DFS(adj_list, counter)); }
    counter++;
  }

  std::sort(result.begin(), result.end());
  return result;
}
