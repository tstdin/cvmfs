/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_SWISSKNIFE_NOTIFY_H_
#define CVMFS_SWISSKNIFE_NOTIFY_H_

#include <string>

#include "swissknife.h"

namespace swissknife {

class CommandNotify : public Command {
 public:
  ~CommandNotify() { }

  virtual std::string GetName() const { return "notify"; }

  virtual std::string GetDescription() const {
    return "Publish the current repository state to a CVMFS notification server\n"
      "This commands publishes the current repository manifest to a CVMFS notification server.";
  }

  ParameterList GetParams() const;

  int Main(const ArgumentList &args);

};

}  // namespace swissknife

#endif  // CVMFS_SWISSKNIFE_NOTIFY_H_
