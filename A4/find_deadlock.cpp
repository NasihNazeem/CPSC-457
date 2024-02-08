// this is the ONLY file you should edit and submit to D2L

#include "find_deadlock.h"
#include "common.h"
#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your
// function
/// needs to stop processing edges and return an instance of Result
/// structure with 'index' set to the index that caused the deadlock, and
/// 'procs' set to contain names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you
/// must return Result with index=-1 and empty procs.
///

/** Steps to accomplishing function:
 *
 * Step 0: Instantiate class and intialize vars
 *
 * Step 1: Macro for loop that checks for condition i < edges.size() to
 * search for each individual edge. This loop then splits the edge string
 * into 3 [process, direction, resource].
 *
 * Step 2: Add this to the graph. To do this convert strings to int for
 process
 * and resource and check whether direction is -> or <-. If -> then
 * adj_list.at(processInt) add the resourceInt. If <-
 adj_list.at(resourceInt)
 * add processInt. out_counts at process/resource Int will increment
 whenever
 * accessed.
 *
 * Step 3: Once added to the graph, we need to sort out the graph.
 *
 *
 *
 *
 *
 *
 */
void print(std::vector<int> const & a);

#define MAX 30000
class FastGraph {
public:
    std::vector<int> adj_list[MAX];
    std::vector<int> out_counts;
    std::string processNames[MAX];
    // to avoid having to convert from Int 2 Word i can just store the names
    // of
    // all processes

    FastGraph(int s) { out_counts.resize(s); }

    Word2Int w2i;

    void addEdge(std::vector<std::string> edge);
    std::vector<std::string> topologicalSort();
};

void FastGraph::addEdge(std::vector<std::string> edge)
{

    std::string processName = edge[0] + ";";
    std::string resourceName = edge[2];
    unsigned int procInt = w2i.get(processName);
    unsigned int resInt = w2i.get(resourceName);

    // add all process names in processNames vector

    if (processNames[procInt].empty()) {
        processNames[procInt] = processName;
    }
    if (processNames[resInt].empty()) {
        processNames[resInt] = resourceName;
    }

    if (edge[1].compare("->")) { // pointing at process
        adj_list[procInt].push_back(resInt);
        out_counts[resInt]++; // out to resource

    } else if (edge[1].compare("<-")) { // pointing at resource
        adj_list[resInt].push_back(procInt); // out to process
        out_counts[procInt]++;
    }
}

std::vector<std::string> FastGraph::topologicalSort()
{
    std::vector<int> out = out_counts;
    std::vector<int> zeroes;

    int j = 0;
    for (auto ele : out) {
        if (ele == 0)
            zeroes.push_back(j);
        j++;
    }

    // main algorithm loop
    while (!zeroes.empty()) {
        // grabs a number from the zeroes vector
        int n = zeroes.back();
        zeroes.pop_back();

        for (auto n2 : adj_list[n]) {
            // removes the edges from each node pointing at the
            // freed node (they get this resource now)
            out[n2]--;
            if (!out[n2]) // adds any nodes that are now zeroes to the vector
                zeroes.push_back(n2);
        }
    }
    std::vector<std::string> sortedV;

    int i = 0;
    for (auto o : out) {

        if (o > 0 && processNames[i].back() == ';') {
            processNames[i].pop_back();
            sortedV.push_back(processNames[i]);
        }
        i++;
    }

    return sortedV;
}

Result find_deadlock(const std::vector<std::string> & edges)
{
    /**
     * Step 1: Macro for loop that checks for condition i < edges.size() to
     * search for each individual edge. This loop then splits the edge string
     * into 3 [process, direction, resource].
     */
    FastGraph G(edges.size());
    Result result;

    /**
     * Step 2
     * Add this to the graph. To do this convert strings to int for process
     * and resource and check whether direction is -> or <-. If -> then
     * adj_list.at(processInt) add the resourceInt. If <-
     * adj_list.at(resourceInt) add processInt. out_counts at
     process/resource
     * Int will increment whenever accessed.
     */
    for (int i = 0; (size_t)i < edges.size(); i++) {
        std::vector<std::string> currEdge;
        currEdge = split(edges[i]);
        G.addEdge(currEdge);
        result.procs = G.topologicalSort();

        if (!result.procs.empty()) {
            result.index = i;
            break;
        } else {
            result.index = -1;
        }
    }

    return result;
}

void print(std::vector<int> const & a)
{
    std::cout << "The vector elements are : ";

    for (size_t i = 0; i < a.size(); i++)
        std::cout << a.at(i) << ' ';
}
