/**
 * This file is part of the CernVM File System.
 */

#include "swissknife_notify.h"

#include <string>

#include "util/string.h"

#include <amqp_tcp_socket.h>
#include "amqp.h"

namespace swissknife {

ParameterList CommandNotify::GetParams() const {
  swissknife::ParameterList r;
  r.push_back(Parameter::Mandatory(
      'r', "CVMFS repository fully-qualified domain name"));
  r.push_back(Parameter::Mandatory('n', "notification server name"));
  r.push_back(Parameter::Mandatory('P', "notification server port"));
  r.push_back(Parameter::Mandatory('u', "notification server username"));
  r.push_back(Parameter::Mandatory('p', "notification server password"));

  return r;
}

int CommandNotify::Main(const swissknife::ArgumentList& args) {
  const std::string repository = *(args.find('r')->second);
  const std::string notif_server_name = *(args.find('n')->second);
  const int notif_server_port = std::atoi(args.find('P')->second->c_str());
  const std::string username = *(args.find('u')->second);
  const std::string password = *(args.find('p')->second);

  LogCvmfs(kLogCvmfs, kLogStdout, "Parameters: ");
  LogCvmfs(kLogCvmfs, kLogStdout, "  CVMFS repository: %s", repository.c_str());
  LogCvmfs(kLogCvmfs, kLogStdout, "  Notification server name: %s",
           notif_server_name.c_str());
  LogCvmfs(kLogCvmfs, kLogStdout, "  Notification server port: %d",
           notif_server_port);
  LogCvmfs(kLogCvmfs, kLogStdout, "  Username: %s", username.c_str());
  //LogCvmfs(kLogCvmfs, kLogStdout, "  Password: %s", password.c_str());

  // Initialization

  amqp_connection_state_t conn = amqp_new_connection();

  amqp_socket_t* socket = amqp_tcp_socket_new(conn);
  if (!socket) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error creating TCP socket.");
    exit(1);
  }

  int status = amqp_socket_open(socket, notif_server_name.c_str(),
                                notif_server_port);
  if (status) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error opening TCP socket.");
    exit(2);
  }

  amqp_rpc_reply_t reply =
      amqp_login(conn, "/cvmfs", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                 username.c_str(), password.c_str());
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error logging in to notification server.");
    exit(3);
  }

  amqp_channel_open(conn, 1);
  reply = amqp_get_rpc_reply(conn);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error opening channel.");
    exit(4);
  }

  // Publish message

  const std::string msg = "Hello";
  amqp_bytes_t msg_bytes;
  msg_bytes.len = msg.size();
  msg_bytes.bytes = (void*)(msg.c_str());

  if (amqp_basic_publish(conn, 1, amqp_cstring_bytes("repository_activity"),
                         amqp_cstring_bytes(repository.c_str()), 0, 0, NULL,
                         msg_bytes)) {
    LogCvmfs(kLogCvmfs, kLogStdout, "Error publishing");
    exit(5);
  }


  // Cleanup

  reply = amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error closing channel.");
    exit(6);
  }

  reply = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error closing connection.");
    exit(7);
  }

  if (amqp_destroy_connection(conn)) {
    LogCvmfs(kLogCvmfs, kLogStderr, "Error ending connection.");
    exit(8);
  }

  return 0;
}

}  // namespace swissknife