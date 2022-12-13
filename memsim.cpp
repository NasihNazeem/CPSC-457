/// -------------------------------------------------------------------------------------
/// this is the only file you need to edit
/// -------------------------------------------------------------------------------------
///
/// (c) 2022, Pavol Federl, pfederl@ucalgary.ca
/// Do not distribute this file.

#include "memsim.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <unordered_map>

struct Partition {
  int tag;
  int64_t size, addr;

  Partition(int tag, int64_t size, int64_t addr)
  {
    this->addr = addr;
    this->size = size;
    this->tag = tag;
  }

  void print()
  {
    std::cout << "Printing partition with tag = " << tag << ":\n";
    std::cout << "size = " << size << std::endl;
    std::cout << "address = " << addr << std::endl;
  }
};

typedef std::list<Partition>::iterator PartitionRef;

struct scmp {
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const
  {
    if (c1->size == c2->size) return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};
// I recommend you implement the simulator as a class. This is only a suggestion.
// If you decide not to use this class, feel free to remove it.
struct Simulator {

  // all partitions linked list
  std::list<Partition> all_blocks;
  // all tagged blocks (blocks that share the same tag will be added to vector or PartitionRefs)
  std::unordered_map<long, std::vector<PartitionRef>> tagged_blocks;

  std::set<PartitionRef, scmp> free_blocks;

  int64_t page_size, pages_req;

  // MemSimResult results;

  Simulator(int64_t page_size)
  {
    // constructor
    this->page_size = page_size;
    pages_req = 0;
  }

  void allocate(int tag, int size)
  {
    // Pseudocode for allocation request:
    // - search through the list of partitions from start to end, and
    //   find the largest partition that fits requested size
    //     - in case of ties, pick the first partition found
    // - if no suitable partition found:
    //     - get minimum number of pages from OS, but consider the
    //       case when last partition is free
    //     - add the new memory at the end of partition list
    //     - the last partition will be the best partition
    // - split the best partition in two if necessary
    //     - mark the first partition occupied, and store the tag in it
    //     - mark the second partition free

    if (all_blocks.empty()) {
      int request_p = 1 + ((size - 1) / page_size);
      int request_b = page_size * request_p;
      pages_req += request_p;

      all_blocks.emplace_back(-1, request_b, 0);
      free_blocks.insert(all_blocks.begin());
    }

    PartitionRef partitions;
    // check if there are any free blocks, if not, add it to the end of all blocks otherwise set to
    // largest partition
    if (free_blocks.empty()) {
      partitions = all_blocks.end();
    } else {
      partitions = *(free_blocks.begin());
    }

    // largest free block is not enough || no free space
    if (size > partitions->size || partitions == all_blocks.end()) {
      int blocks_requested;
      int pages_requested;

      // Check end block if free mem, if so add minimum amount to the current size of block.
      // If not (else), make a new partition
      if (all_blocks.back().tag == -1) {
        partitions = all_blocks.end();
        --partitions;
        free_blocks.erase(partitions);
        pages_requested = 1 + ((size - all_blocks.back().size - 1) / page_size);
        blocks_requested = pages_requested * page_size;
        all_blocks.back().size += blocks_requested;
        free_blocks.insert(partitions);
      } else {
        pages_requested = 1 + ((size - 1) / page_size);
        blocks_requested = pages_requested * page_size;
        // add to back, -1 tag, minimum blocks and address offset by size requested
        all_blocks.emplace_back(
            -1, blocks_requested, (all_blocks.back().addr + all_blocks.back().size));
        free_blocks.insert(std::prev(all_blocks.end())); // free_blocks new partition

        partitions = all_blocks.end();
        --partitions;
      }

      pages_req += pages_requested; // update pages requested
    }

    // size of partition is a perfect fit, so we:
    // erase partition from free_blocks
    // update tag to not be free
    // update tagged_blocks map where we insert partition into vector belonging to tag

    // OR

    // we create a new partition with for space
    // where we insert partition into all_blocks
    // erase that partitions temporarily from free_blocks
    // update partition addr and size after erasure
    // merge partition with next block if free
    if (size == partitions->size) {
      free_blocks.erase(partitions);
      partitions->tag = tag;
      tagged_blocks[tag].push_back(partitions);
    } else if (size < partitions->size) {
      all_blocks.insert(partitions, Partition(tag, size, partitions->addr));

      free_blocks.erase(partitions);
      partitions->addr += size;
      partitions->size -= size;

      if (partitions != all_blocks.end()) {
        if (std::next(partitions)->tag == -1) { // check if free block
          partitions->size += std::next(partitions)->size; // add sizes
          free_blocks.erase(std::next(partitions));
          all_blocks.erase(
              std::next(partitions)); // erase from both set and list after merge and data update
        }
      }

      tagged_blocks[tag].push_back(
          std::prev(partitions)); // tag newly created partition to a vector
      free_blocks.insert(partitions); // add the free partition once again
    }
  }
  void deallocate(int tag)
  {
    // Pseudocode for deallocation request:
    // - for every partition
    //     - if partition is occupied and has a matching tag:
    //         - mark the partition free
    //         - merge any adjacent free partitions

    std::vector<PartitionRef> partitions = tagged_blocks[tag];

    for (PartitionRef i : partitions) {
      i->tag = -1;
      // free block

      PartitionRef nextBlock = std::next(i);
      if (nextBlock->tag == -1) {
        // Remove block from all_blocks and free_blocks and merge next block to current if its a
        // free block (tag == -1)
        i->size += nextBlock->size;
        all_blocks.erase(nextBlock);
        free_blocks.erase(nextBlock);
      }

      nextBlock = std::prev(i);

      if (nextBlock->tag == -1) {
        // Remove block from all_blocks and free_blocks and merge left block to current if its a
        // free block (tag == -1). Also update the address to the left most block's address
        i->size += nextBlock->size;
        i->addr = nextBlock->addr;
        all_blocks.erase(nextBlock);
        free_blocks.erase(nextBlock);
      }

      free_blocks.insert(i); // after merges, add block into free mem list
    }

    tagged_blocks.erase(tag); // remove all blocks with this tag from this list
  }

  MemSimResult getStats()
  {
    MemSimResult result;
    result.n_pages_requested = pages_req;

    PartitionRef partition = *(free_blocks.begin());
    if (! free_blocks.empty()) {
      // update MemSimResult addr and size with largest free partition if available
      result.max_free_partition_address = partition->addr;
      result.max_free_partition_size = partition->size;
    } else {
      result.max_free_partition_address = 0;
      result.max_free_partition_size = 0;
      // set to 0 if no free_blocks
    }

    return result;
  }
  void check_consistency()
  {
    // mem_sim() calls this after every request to make sure all data structures
    // are consistent. Since this will probablly slow down your code, you should
    // disable comment out the call below before submitting your code for grading.
    check_consistency_internal();
  }
  void check_consistency_internal()
  {
    // you do not need to implement this method at all - this is just my suggestion
    // to help you with debugging

    // here are some suggestions for consistency checks (see appendix also):

    // make sure the sum of all partition sizes in your linked list is
    // the same as number of page requests * page_size

    // make sure your addresses are correct

    // make sure the number of all partitions in your tag data structure +
    // number of partitions in your free blocks is the same as the size
    // of the linked list

    // make sure that every free partition is in free blocks

    // make sure that every partition in free_blocks is actually free

    // make sure that none of the partition sizes or addresses are < 1

    printf("CONSISTENCY CHECK:\n");
    PartitionRef p;
    // make sure the sum of all partition sizes in your linked list is
    // the same as number of page requests * page_size
    int64_t size_sum = 0;
    for (p = all_blocks.begin(); p != all_blocks.end(); ++p) {
      size_sum += p->size;
      printf("partition: addr=%ld, size=%ld, tag=%d\n", p->addr, p->size, p->tag); // debug
    }
    if (size_sum == pages_req * page_size) {
      printf("SUM CHECK PASSED: partition size equals page requests\n");
    } else {
      printf(
          "!!!SUM CHECK FAILED: size_sum = %ld, requests * size = %ld\n",
          size_sum,
          (pages_req * page_size));
      exit(-1);
    }

    // make sure your addresses are correct
    int64_t failed_addr = -1;
    for (p = all_blocks.begin(); p != all_blocks.end(); ++p) {
      if (p == all_blocks.begin()) { // at start, checks if it's 0
        if (p->addr != 0) {
          failed_addr = 0;
          break;
        }
      } else { // otherwise, checks if address is correct
        if (p->addr != std::prev(p)->addr + std::prev(p)->size) {
          failed_addr = p->addr;
          break;
        }
      }
    }
    if (failed_addr == -1) {
      printf("ADDRESS CHECK PASSED: addresses correct\n");
    } else {
      printf("!!!ADDRESS CHECK FAILED: first failed address at %ld\n", failed_addr);
      exit(-1);
    }

    // make sure that every free partition is in free blocks

    // make sure that every partition in free_blocks is actually free

    // make sure that none of the partition sizes or addresses are < 1
    bool pass = true;
    for (p = all_blocks.begin(); p != all_blocks.end(); ++p) {
      if (p->size < 1 || p->addr < 0) {
        pass = false;
        break;
      }
    }
    if (pass) {
      printf("MEMBER CHECK PASSED: no invalid size/address\n");
    } else {
      printf("!!!MEMBER CHECK FAILED: negative address or negative/zero size found.\n");
      exit(-1);
    }
  }
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // if you decide to use the simulator class above, you probably do not need
  // to modify below at all
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    // sim.check_consistency();
  }
  return sim.getStats();
}
