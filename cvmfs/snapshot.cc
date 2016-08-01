/**
 * This file is part of the CernVM File System.
 */

#include "cvmfs_config.h"
#include "snapshot.h"

#include "catalog_mgr_client.h"

using namespace std;  // NOLINT

const char *SnapshotRegistry::kPathPrefix = "/.cvmfs_snapshots";


SnapshotRegistry::SnapshotRegistry(
  catalog::ClientCatalogManager *catalog_mgr_head)
  : catalog_mgr_head_(catalog_mgr_head)
  , path_prefix_(std::string(kPathPrefix))
  , highest_inode_(catalog_mgr_head_->all_inodes() * 5)
{

}
