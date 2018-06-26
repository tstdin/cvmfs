/**
 * This file is part of the CernVM File System.
 */

#include "swissknife_notify.h"

#include <string>

#include "download.h"
#include "manifest.h"
#include "util/posix.h"
#include "util/string.h"

#include "amqp.h"
#include "amqp_tcp_socket.h"

namespace swissknife {

ParameterList CommandNotify::GetParams() const {
  swissknife::ParameterList r;
  r.push_back(Parameter::Mandatory('r', "CVMFS repository URL"));
  r.push_back(Parameter::Mandatory('n', "Notification server URL"));
  r.push_back(Parameter::Mandatory('u', "Notification server username"));
  r.push_back(Parameter::Mandatory('p', "Notification server password"));
  r.push_back(Parameter::Switch('L', "follow HTTP redirects"));

  return r;
}

int CommandNotify::Main(const swissknife::ArgumentList& args) {
  const std::string repository_url =
      MakeCanonicalPath(*(args.find('r')->second));
  std::string notif_server_url = *(args.find('n')->second);
  const std::string username = *(args.find('u')->second);
  const std::string password = *(args.find('p')->second);

  LogCvmfs(kLogCvmfs, kLogStdout, "Parameters: ");
  LogCvmfs(kLogCvmfs, kLogStdout, "  CVMFS repository URL: %s",
           repository_url.c_str());
  LogCvmfs(kLogCvmfs, kLogStdout, "  Notification server URL: %s",
           notif_server_url.c_str());
  LogCvmfs(kLogCvmfs, kLogStdout, "  Username: %s", username.c_str());
  // LogCvmfs(kLogCvmfs, kLogStdout, "  Password: %s", password.c_str());

  // Initialization

  amqp_connection_state_t conn = amqp_new_connection();

  amqp_socket_t* socket = amqp_tcp_socket_new(conn);
  if (!socket) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error creating TCP socket.");
    return 1;
  }

  if (HasPrefix(notif_server_url, "http://", false)) {
    notif_server_url.erase(notif_server_url.begin(),
                           notif_server_url.begin() + 7);
  }

  std::vector<std::string> url_tokens = SplitString(notif_server_url, ':');
  assert(url_tokens.size() > 0);

  const std::string notif_server_name = url_tokens[0];
  const int notif_server_port =
      url_tokens.size() > 1 ? std::atoi(url_tokens[1].c_str()) : 5672;
  int status =
      amqp_socket_open(socket, notif_server_name.c_str(), notif_server_port);
  if (status) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error opening TCP socket.");
    return 2;
  }

  amqp_rpc_reply_t reply =
      amqp_login(conn, "/cvmfs", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                 username.c_str(), password.c_str());
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error logging in to notification server.");
    return 3;
  }

  amqp_channel_open(conn, 1);
  reply = amqp_get_rpc_reply(conn);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error opening channel.");
    return 4;
  }

  // Extract repository name from repository URL
  const std::vector<std::string> repo_url_tokens =
      SplitString(repository_url, '/');
  const std::string repository_name = repo_url_tokens.back();

  // Download repository manifest
  std::string manifest_contents;
  const std::string manifest_url = repository_url + "/.cvmfspublished";
  if (HasPrefix(repository_url, "http://", false)) {
    const bool follow_redirects = args.count('L') > 0;
    if (!this->InitDownloadManager(follow_redirects)) {
      LogCvmfs(kLogCvmfs, kLogStderr, "Could not initialize download manager");
      return 5;
    }
    download::JobInfo download_manifest(&manifest_url, false, false, NULL);
    download::Failures retval = download_manager()->Fetch(&download_manifest);
    if (retval != download::kFailOk) {
      LogCvmfs(kLogCvmfs, kLogStderr, "Failed to download manifest (%d - %s)",
               retval, download::Code2Ascii(retval));
      return 6;
    }
    char* buffer = download_manifest.destination_mem.data;
    const unsigned length = download_manifest.destination_mem.pos;
    manifest_contents = std::string(download_manifest.destination_mem.data,
                                    download_manifest.destination_mem.pos);
    free(download_manifest.destination_mem.data);
  } else {
    int fd = open(manifest_url.c_str(), O_RDONLY);
    if (fd == -1) {
      LogCvmfs(kLogCvmfs, kLogStdout, "Could not open manifest file");
      return 7;
    }
    if (!SafeReadToString(fd, &manifest_contents)) {
      LogCvmfs(kLogCvmfs, kLogStdout, "Could not read manifest file");
      close(fd);
      return 8;
    }
    close(fd);
  }

  UniquePtr<manifest::Manifest> manifest(manifest::Manifest::LoadMem(
      reinterpret_cast<const unsigned char*>(manifest_contents.data()),
      manifest_contents.size()));

  LogCvmfs(kLogCvmfs, kLogStdout, "Current repository manifest:\n%s",
           manifest->ExportString().c_str());


  // Publish message

  const std::string msg = Base64(manifest_contents);
  amqp_bytes_t msg_bytes;
  msg_bytes.len = msg.size();
  msg_bytes.bytes = (void*)(msg.c_str());

  if (amqp_basic_publish(conn, 1, amqp_cstring_bytes("repository_activity"),
                         amqp_cstring_bytes(repository_name.c_str()), 0, 0,
                         NULL, msg_bytes)) {
    LogCvmfs(kLogCvmfs, kLogStdout, "Error publishing");
    return 9;
  }

  // Cleanup

  reply = amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error closing channel.");
    return 10;
  }

  reply = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error closing connection.");
    return 11;
  }

  if (amqp_destroy_connection(conn)) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error ending connection.");
    return 12;
  }

  return 0;
}

}  // namespace swissknife