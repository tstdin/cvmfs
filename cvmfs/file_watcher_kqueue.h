/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_FILE_WATCHER_KQUEUE_H_
#define CVMFS_FILE_WATCHER_KQUEUE_H_

#include "file_watcher.h"

#include <map>
#include <string>

namespace file_watcher {

class FileWatcherKqueue : public FileWatcher {
 public:
  FileWatcherKqueue();
  virtual ~FileWatcherKqueue();

 protected:
  virtual bool RunEventLoop(const FileWatcher::HandlerMap& handler,
                            int read_pipe, int write_pipe);

 private:
  void RemoveFilter(int fd);
  void RegisterFilter(const std::string& file_path,
                      EventHandler* handler);

  int kq_;
  std::map<int, WatchRecord> watch_records_;
};

}  // namespace file_watcher

#endif  // CVMFS_FILE_WATCHER_KQUEUE_H_
