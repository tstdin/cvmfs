/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_SNAPSHOT_H_
#define CVMFS_SNAPSHOT_H_

#include "inttypes.h"

#include <ctime>
#include <string>
#include <vector>

#include "hash.h"
#include "shortstring.h"

class MountPoint;

class SnapshotRegistry {
 public:
   explicit SnapshotRegistry(MountPoint *mountpoint_head)
     : mountpoint_head_(mountpoint_head) { }
   MountPoint *Inode2MountPoint(uint64_t inode);
   MountPoint *Path2MountPoint(const PathString &path);

 private:
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
  std::vector<SnapshotInfo> snapshots_;
};  // class MountPointRegistry

#endif  // CVMFS_SNAPSHOT_H_
