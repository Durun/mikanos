#include "fat.hpp"

#include <cstring>

namespace fat {

// #@@range_begin(init_fat)
BPB* boot_volume_image;

void Initialize(void* volume_image) {
  boot_volume_image = reinterpret_cast<fat::BPB*>(volume_image);
}
// #@@range_end(init_fat)

// #@@range_begin(get_cluster_addr)
uintptr_t GetClusterAddr(unsigned long cluster) {
  unsigned long sector_num =
    boot_volume_image->reserved_sector_count +
    boot_volume_image->num_fats * boot_volume_image->fat_size_32 +
    (cluster - 2) * boot_volume_image->sectors_per_cluster;
  uintptr_t offset = sector_num * boot_volume_image->bytes_per_sector;
  return reinterpret_cast<uintptr_t>(boot_volume_image) + offset;
}
// #@@range_end(get_cluster_addr)

// #@@range_begin(read_name)
void ReadName(const DirectoryEntry& entry, char* base, char* ext) {
  memcpy(base, &entry.name[0], 8);
  base[8] = 0;
  for (int i = 7; i >= 0 && base[i] == 0x20; --i) {
    base[i] = 0;
  }

  memcpy(ext, &entry.name[8], 3);
  ext[3] = 0;
  for (int i = 2; i >= 0 && ext[i] == 0x20; --i) {
    ext[i] = 0;
  }
}
// #@@range_end(read_name)
void ReadLongName(const DirectoryEntry entries[], int i, char name[]) {
  name[0] = 0;
  const uint8_t LAST_LONG_ENTRY = 0x40;
  if (i <= 0) return;
  if (entries[i - 1].attr != Attribute::kLongName) return;

  LFNDirectoryEntry* entry = (LFNDirectoryEntry*) &entries[i - 1];
  if (entry->LDIR_Type != 0) return;
  if (entry->LDIR_FstClusLO[0] != 0) return;
  if (entry->LDIR_FstClusLO[1] != 0) return;

  uint8_t ord = entry->LDIR_Ord;
  char16_t longName[14];
  if ((ord == 0x01) || (ord & LAST_LONG_ENTRY)) {
    for (int i = 0; i < 5; i++) longName[i] = entry->LDIR_Name1[i];
    for (int i = 0; i < 6; i++) longName[5 + i] = entry->LDIR_Name2[i];
    for (int i = 0; i < 2; i++) longName[11 + i] = entry->LDIR_Name3[i];
  }
  longName[13] = 0;
  // return
  String16toString8(longName, name);
}
void String16toString8(char16_t from[], char to[]) {
  int i = 0;
  char *c;
  do {
    c = (char*)(&(from[i]));
    to[i] = *c;
    i++;
  } while (*c != 0);
}

}  // namespace fat
