/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_SNAPSHOT_H_
#define CVMFS_SNAPSHOT_H_

#include <inttypes.h>

#include <cassert>
#include <ctime>
#include <string>
#include <vector>

#include "hash.h"
#include "shortstring.h"

namespace catalog {
class ClientCatalogManager;
}

class SnapshotRegistry {
 public:
  explicit SnapshotRegistry(catalog::ClientCatalogManager *catalog_mgr_head);

 private:
  static const char *kPathPrefix;  // "/.cvmfs_snapshots"

  struct SnapshotInfo {
    SnapshotInfo(const std::string &n, const shash::Any &r, time_t t)
      : tag_name(n)
      , root_hash(r)
      , timestamp(t)
      , catalog_mgr(NULL)
      { }
    std::string tag_name;
    shash::Any root_hash;
    time_t timestamp;
    catalog::ClientCatalogManager *catalog_mgr;
  };

  catalog::ClientCatalogManager *catalog_mgr_head_;
  std::vector<SnapshotInfo> snapshots_;
  PathString path_prefix_;
  uint64_t highest_inode_;
};  // class MountPointRegistry

#endif  // CVMFS_SNAPSHOT_H_
