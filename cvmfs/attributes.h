/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_ATTRIBUTES_H_
#define CVMFS_ATTRIBUTES_H_

#include "util/single_copy.h"

/**
 * Keeps track of information about the client and the mounted repositories that
 * are exported to cvmfs users.  The attributes are stored in _CVMFS_ATTR_<XYZ>
 * environment variables.  They are accessible through variant symlinks in
 * /.cvmfs/attributes/<XYZ>.
 *
 * TODO(jblomer): replace extended attributes and talk code by calls into the
 * attribute manager.
 */
class AttributeManager : SingleCopy {
 public:
 private:
};  // class AttributeManager

#endif  // CVMFS_ATTRIBUTES_H_
