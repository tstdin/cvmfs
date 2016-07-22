/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_SNAPSHOT_H_
#define CVMFS_SNAPSHOT_H_

#include "inttypes.h"

#include <cassert>
#include <ctime>
#include <string>
#include <vector>

#include "hash.h"
#include "shortstring.h"

class MountPoint;

class SnapshotRegistry {
 public:
  struct Tag {
    friend class SnapshotRegistry;
   public:
     Tag() : tag_(kTagInvalid) { }
     explicit Tag(int32_t t) : tag_(t) { }
     bool IsSnapshot() { return tag_ >= 0; }
     bool IsHead() { return tag_ == kTagHead; }
     bool IsVirtual() { return tag_ == kTagVirtual; }
     bool IsInvalid() { return tag_ == kTagInvalid; }
     unsigned ToIdx() {
       assert(IsSnapshot());
       return tag_;
     }

   private:
    const static int32_t kTagHead = -1;
    const static int32_t kTagVirtual = -2;
    const static int32_t kTagInvalid = -3;
    int32_t tag_;
  };

  explicit SnapshotRegistry(MountPoint *mountpoint_head)
    : mountpoint_head_(mountpoint_head)
    , snapshots_(NULL)
    , num_snapshots_(0) { }

  Tag Inode2Tag(uint64_t inode) {
    uint64_t tag_in_inode = (inode & kInodeMask) >> kInodeBits;
    uint32_t tag = tag_in_inode;
    switch (tag) {
      case 0:
        return Tag(Tag::kTagHead);
      case 1:
        return Tag(Tag::kTagVirtual);
      default:
        uint32_t idx = tag - 2;
        return (idx >= num_snapshots_) ? Tag(Tag::kTagInvalid) : Tag(idx);
    }
  }


  Tag Path2Tag(const PathString &path) {
    // TODO(jblomer)
    return Tag(Tag::kTagInvalid);
  }

  MountPoint *Tag2MountPoint(Tag tag) {
    if (tag.IsHead())
      return mountpoint_head_;
    return snapshots_[tag.ToIdx()].mount_point;
  }

 private:
  static const unsigned kInodeBits = 48;
  static const uint64_t kInodeMask = 0x0000FFFFFFFFFFFFull;  // upper 16 bits
  static const char *kPathPrefix;  // "/.cvmfs_snapshots"

  struct SnapshotInfo {
    SnapshotInfo(const std::string &n, const shash::Any &r, time_t t)
      : tag_name(n)
      , root_hash(r)
      , timestamp(t)
      , mount_point(NULL)
      { }
    std::string tag_name;
    shash::Any root_hash;
    time_t timestamp;
    MountPoint *mount_point;
  };

  MountPoint *mountpoint_head_;
  SnapshotInfo *snapshots_;
  unsigned num_snapshots_;
};  // class MountPointRegistry

#endif  // CVMFS_SNAPSHOT_H_
