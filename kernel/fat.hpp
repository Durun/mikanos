/**
 * @file fat.hpp
 *
 * FATファイルシステムを操作するためのプログラムを集めたファイル。
 */

#pragma once

#include <cstdint>

namespace fat {

struct BPB {
  uint8_t jump_boot[3];
  char oem_name[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sector_count;
  uint8_t num_fats;
  uint16_t root_entry_count;
  uint16_t total_sectors_16;
  uint8_t media;
  uint16_t fat_size_16;
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;
  uint32_t fat_size_32;
  uint16_t ext_flags;
  uint16_t fs_version;
  uint32_t root_cluster;
  uint16_t fs_info;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  char volume_label[11];
  char fs_type[8];
} __attribute__((packed));

enum class Attribute : uint8_t {
  kReadOnly  = 0x01,
  kHidden    = 0x02,
  kSystem    = 0x04,
  kVolumeID  = 0x08,
  kDirectory = 0x10,
  kArchive   = 0x20,
  kLongName  = 0x0f,
};

struct DirectoryEntry {
  unsigned char name[11];
  Attribute attr;
  uint8_t ntres;
  uint8_t create_time_tenth;
  uint16_t create_time;
  uint16_t create_date;
  uint16_t last_access_date;
  uint16_t first_cluster_high;
  uint16_t write_time;
  uint16_t write_date;
  uint16_t first_cluster_low;
  uint32_t file_size;

  uint32_t FirstCluster() const {
    return first_cluster_low |
      (static_cast<uint32_t>(first_cluster_high) << 16);
  }
} __attribute__((packed));

struct LFNDirectoryEntry {
  // 解説は http://elm-chan.org/docs/fat.html#lfn より
  uint8_t LDIR_Ord;           // このエントリがLFNエントリ(1個のLFNを構成するエントリ群)のどの部分かを示すシーケンス番号(1～20)。1がLFNの先頭部を意味する。LAST_LONG_ENTRYフラグ(0x40)が立っているときは、LFNエントリの開始であることを示す。
  char16_t LDIR_Name1[5];     // 名前。1文字目～5文字目。Unicode(UTF-16LE)で格納される。
  uint8_t LDIR_Attr;          // LFNアトリビュート。このエントリがLFNエントリの一部であることを示すため、ATTR_LONG_NAMEでなければならない。
  uint8_t LDIR_Type;          // LFNのタイプ。常に0でなければならず、0以外は予約。
  uint8_t LDIR_Chksum;        // このLFNエントリと結びつけられているSFNエントリのチェックサム。
  char16_t LDIR_Name2[6];     // 名前。6文字目～11文字目。
  uint8_t LDIR_FstClusLO[2];  // 古いディスクユーティリティによる危険の可能性を避けるため、0がセットされる。
  char16_t LDIR_Name3[2];     // 名前。12文字目～13文字目。
} __attribute__((packed));

extern BPB* boot_volume_image;
void Initialize(void* volume_image);

/** @brief 指定されたクラスタの先頭セクタが置いてあるメモリアドレスを返す。
 *
 * @param cluster  クラスタ番号（2 始まり）
 * @return クラスタの先頭セクタが置いてあるメモリ領域のアドレス
 */
uintptr_t GetClusterAddr(unsigned long cluster);

/** @brief 指定されたクラスタの先頭セクタが置いてあるメモリ領域を返す。
 *
 * @param cluster  クラスタ番号（2 始まり）
 * @return クラスタの先頭セクタが置いてあるメモリ領域へのポインタ
 */
// #@@range_begin(get_sector)
template <class T>
T* GetSectorByCluster(unsigned long cluster) {
  return reinterpret_cast<T*>(GetClusterAddr(cluster));
}
// #@@range_end(get_sector)

/** @brief ディレクトリエントリの短名を基本名と拡張子名に分割して取得する。
 * パディングされた空白文字（0x20）は取り除かれ，ヌル終端される。
 *
 * @param entry  ファイル名を得る対象のディレクトリエントリ
 * @param base  拡張子を除いたファイル名（9 バイト以上の配列）
 * @param ext  拡張子（4 バイト以上の配列）
 */
void ReadName(const DirectoryEntry& entry, char* base, char* ext);

/** @brief ディレクトリエントリの長名を取得する。長名が無効であればヌル文字で始まる文字列を返す。
 *
 * @param entries
 * @param i
 * @param name  長名（size14以上の配列）
 */
void ReadLongName(const DirectoryEntry entries[], int i, char name[]);
void String16toString8(char16_t from[], char to[]);
} // namespace fat
