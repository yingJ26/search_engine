/*
 * Copyright ©2020 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Summer Quarter 2020 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}
namespace hw4 {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int *listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd".

  // STEP 1:
  // inspired from lecture code server_bind_listen.cc
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = ai_family;
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use wildcard "in6addr_any" address
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  int listen_fd_temp;

  struct addrinfo *result;
  int res = getaddrinfo(nullptr, std::to_string
  (port_).c_str(), &hints, &result);

  if (res != 0) {
      return false;
  }

  // Loop through the returned address structures until we are able
  // to create a socket and bind to one.  The address structures are
  // linked in a list through the "ai_next" field of result.
  for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
      listen_fd_temp = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if (listen_fd_temp == -1) {
          // Creating this socket failed.  So, loop to the next returned
          // result and try again.
          continue;
      }

      // Configure the socket; we're setting a socket "option."  In
      // particular, we set "SO_REUSEADDR", which tells the TCP stack
      // so make the port we bind to available again as soon as we
      // exit, rather than waiting for a few tens of seconds to recycle it.
      int optval = 1;
      setsockopt(listen_fd_temp, SOL_SOCKET, SO_REUSEADDR,
          &optval, sizeof(optval));

      // Try binding the socket to the address and port number returned
      // by getaddrinfo().
      if (bind(listen_fd_temp, rp->ai_addr, rp->ai_addrlen) == 0) {
          sock_family_ = rp->ai_family;
          break;
      }

      // The bind failed.  Close the socket, then loop back around and
      // try the next address/port returned by getaddrinfo().
      close(listen_fd_temp);
  }
  freeaddrinfo(result);

  // fail to bind to any address
  if (listen_fd_temp == -1) {
      return false;
  }

  // else, create a listening socket
  if (listen(listen_fd_temp, SOMAXCONN) != 0) {
      close(listen_fd_temp);
      return false;
  }
  // on success
  *listen_fd = listen_fd_temp;
  listen_sock_fd_ = listen_fd_temp;
  return true;
}

bool ServerSocket::Accept(int *accepted_fd,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dnsname,
                          std::string *server_addr,
                          std::string *server_dnsname) {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // STEP 2:
  // inspired by the lecture code
  while (1) {
      struct sockaddr_storage caddr;
      socklen_t addr_len = sizeof(caddr);
      int client_fd = accept(listen_sock_fd_,
          reinterpret_cast<struct sockaddr *>(&caddr), &addr_len);
      if (client_fd < 0) {
          if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)) {
              continue;
          }
          return false;
      }

      *accepted_fd = client_fd;

      sockaddr* addr = reinterpret_cast<struct sockaddr *>(&caddr);

      if (addr->sa_family == AF_INET) {
          // if IPv4
          char astring[INET_ADDRSTRLEN];
          struct sockaddr_in *in4 = reinterpret_cast
              <struct sockaddr_in *>(addr);

          inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
          *client_addr = std::string(astring);
          *client_port = ntohs(in4->sin_port);
      } else if (addr->sa_family == AF_INET6) {
          // if IPv6
          char astring[INET6_ADDRSTRLEN];
          struct sockaddr_in6 *in6 = reinterpret_cast
              <struct sockaddr_in6 *>(addr);

          inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
          *client_addr = std::string(astring);
          *client_port = ntohs(in6->sin6_port);
      } else {
          return false;
      }

      char hostname[1024];
      if (getnameinfo(addr, addr_len, hostname, 1024, nullptr, 0, 0) != 0) {
          return false;
      }
      *client_dnsname = std::string(hostname);

      char hname[1024];
      hname[0] = '\0';

      if (sock_family_ == AF_INET) {
          // The server is using an IPv4 address.
          struct sockaddr_in srvr;
          socklen_t srvrlen = sizeof(srvr);
          char addrbuf[INET_ADDRSTRLEN];
          getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
          inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
          *server_addr = std::string(addrbuf);
          // Get the server's dns name, or return it's IP address as
          // a substitute if the dns lookup fails.
          getnameinfo((const struct sockaddr *) &srvr,
              srvrlen, hname, 1024, nullptr, 0, 0);
          *server_dnsname = std::string(hname);
      } else {
          // The server is using an IPv6 address.
          struct sockaddr_in6 srvr;
          socklen_t srvrlen = sizeof(srvr);
          char addrbuf[INET6_ADDRSTRLEN];
          getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
          inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
          *server_addr = std::string(addrbuf);
          // Get the server's dns name, or return it's IP address as
          // a substitute if the dns lookup fails.
          getnameinfo((const struct sockaddr *) &srvr,
              srvrlen, hname, 1024, nullptr, 0, 0);
          *server_dnsname = std::string(hname);
      }
      break;
  }
  return true;
}

}  // namespace hw4
