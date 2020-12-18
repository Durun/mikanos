#include "paging.hpp"

#include <array>

#include "asmfunc.h"

// #@@range_begin(setup_page)
namespace {
  const uint64_t kPageSize4K = 4096;
  const uint64_t kPageSize2M = 512 * kPageSize4K;
  const uint64_t kPageSize1G = 512 * kPageSize2M;

  alignas(kPageSize4K) std::array<uint64_t, 512> pml4_table;
  alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
  alignas(kPageSize4K)
    std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;

  uint64_t toPresent(uint64_t entry) {
      return entry | 0b0001;
  }
  uint64_t toWritable(uint64_t entry) {
      return entry | 0b0010;
  }
  uint64_t toHugePage(uint64_t entry) {
      return entry | 0b1000'0000;
  }
}


void SetupIdentityPageTable() {
  pml4_table[0] = toWritable(toPresent(reinterpret_cast<uint64_t>(&pdp_table[0])));
  for (int i_pdpt = 0; i_pdpt < page_directory.size(); ++i_pdpt) {
    pdp_table[i_pdpt] = toWritable(toPresent(reinterpret_cast<uint64_t>(&page_directory[i_pdpt])));
    for (int i_pd = 0; i_pd < 512; ++i_pd) {
        page_directory[i_pdpt][i_pd] = toWritable(toPresent(toHugePage(i_pdpt * kPageSize1G + i_pd * kPageSize2M)));
    }
  }

  SetCR3(reinterpret_cast<uint64_t>(&pml4_table[0]));
}
// #@@range_end(setup_page)
