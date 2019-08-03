//
// Created by S on 2019/8/2.
//

#ifndef _MGR_H_
#define _MGR_H_

#include "common.h"
#include "conn.h"

class host {
 public:
  char mHostName[HOST_NAME_SIZE];
  int mPort;
  int mConnect;
};

class mgr {
 public:
  mgr( );
  ~mgr();
 private:
  static int mEpollFd;
  map<int, conn *> mCons;
  map<int, conn *> mUsed;
  map<int, conn *> mFreed;
};

#endif //_MGR_H_
