//
// Created by S on 2019/8/2.
//

#ifndef _MGR_H_
#define _MGR_H_

#include "common.h"
#include "conn.h"
#include "fdwrapper.h"

class host {
 public:
  char mHostName[HOST_NAME_SIZE];
  int mPort;
  int mConnect;
};

class mgr {
 public:
  mgr (int epollFd, const host &srv);
  ~mgr ();
 public:
  int conn2srv (const struct sockaddr_in &address);
  conn* pickConn(int connFd);
  void freeConn( conn* connection );
  int getUsedConnCnt();
  void recycleConns();
  RET_CODE process( int fd, OP_TYPE type );
 private:
  static int mEpollFd;
  map<int, conn *> mCons;
  map<int, conn *> mUsed;
  map<int, conn *> mFreed;
  host m_logic_srv;
};

#endif //_MGR_H_
