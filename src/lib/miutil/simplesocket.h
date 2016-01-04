#ifndef __SimpleSocket_h__
#define __SimpleSocket_h__

#include "sockethelper.h"

namespace miutil {
class SimpleSocket {
  SimpleSocket(const SimpleSocket &);
  SimpleSocket& operator=(const SimpleSocket &);

 protected:
  int fd;
  bool mConnected;
  bool timeout_;

  void setfd(int fd);

 public:
  SimpleSocket();
  SimpleSocket(int fd_);
  ~SimpleSocket();

  void close();
  int getfd() const {
    return fd;
  }
  bool connected() const {
    return mConnected;
  }
  ;
  int getCh(char &ch, bool &timedout, int timeoutInSec = 10);
  int getChExt(char &ch, bool &timedout, int timeoutInMilliSec = 10000);
  int writeBuf(const std::string &str);
  int writeln(const std::string &str);
  int readln(std::string &str, int timeoutInSecBetweenReceivedBytes = 10);
  int readlnExt(std::string &str, int timeoutInMilliSecBetweenReceivedBytes =
                    10000);

  bool setLinger(bool enable, int timeoutInSec);
  bool getLinger(bool &enabled, int &timeoutInSec);
  bool timeout() const {
    return timeout_;
  }
  ;

};

class SimpleSocketClient : public SimpleSocket {
  SimpleSocketClient(const SimpleSocketClient&);
  SimpleSocketClient& operator=(const SimpleSocketClient&);

 private:
  socket_status_codes::ErrorTypes mErrorType;

 public:
  SimpleSocketClient();
  SimpleSocketClient(const std::string &ervername, int port);
  ~SimpleSocketClient();

  socket_status_codes::ErrorTypes errorType() {
    return mErrorType;
  }
  bool connect(const std::string &servername, int port);
};

class SimpleSocketServer : public SimpleSocket {
  SimpleSocketServer(const SimpleSocketServer &);
  SimpleSocketServer& operator=(const SimpleSocketServer &);

  socket_status_codes::ErrorTypes mErrorType;
  int port;

 public:
  SimpleSocketServer();
  ~SimpleSocketServer();

  socket_status_codes::ErrorTypes errorType() {
    return mErrorType;
  }

  bool bind(int port);
  bool listen(int backlog);
  int getPort() const {
    return port;
  }
  SimpleSocket* accept(bool &interupted, int timeout = 1);
};
}
#endif
