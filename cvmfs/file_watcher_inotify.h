/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_FILE_WATCHER_INOTIFY_H_
#define CVMFS_FILE_WATCHER_INOTIFY_H_

#include "file_watcher.h"

#include <map>
#include <string>

namespace file_watcher {

class FileWatcherInotify : public FileWatcher {
 public:
  FileWatcherInotify();
  virtual ~FileWatcherInotify();

 protected:
  virtual bool RunEventLoop(const FileWatcher::HandlerMap& handler,
                            int read_pipe, int write_pipe);

 private:
  void RegisterFilter(const std::string& file_path,
                      EventHandler* handler);

  int inotify_fd_;
  std::map<int, WatchRecord> watch_records_;
};

}  // namespace file_watcher

#endif  // CVMFS_FILE_WATCHER_INOTIFY_H_
