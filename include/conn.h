//
// Created by S on 2019/8/3.
//

#ifndef _CONN_H_
#define _CONN_H_

#include "common.h"
#include "fdwrapper.h"

class conn {
 public:
  conn();
  ~conn ();
 public:
  void reset();
 public:
  static const int BUF_SIZE = 2048;

  char *mCliBuf;
  int mCliReadIdx;
  int mCliWriteIdx;
  struct sockaddr_in mCliAddr;
  int mCliFd;

  char *mSerBuf;
  int mSerReadIdx;
  int mSerWriteIdx;
  struct sockaddr_in mSerAddr;
  int mSerFd;

  bool mSerClosed;
};

#endif //_CONN_H_
