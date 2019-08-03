//
// Created by S on 2019/8/3.
//

#include "conn.h"


conn::~conn ()
{
  delete[]mCliBuf;
  delete[]mSerBuf;
}

conn::conn ()
{
  mSerFd = -1;
  mCliBuf = new char[BUF_SIZE];
  mSerBuf == new char[BUF_SIZE];
  if (mCliBuf == nullptr || mSerBuf == nullptr)
    {
      ERR_EXIT ("conn");
    }
  reset ();
}

void conn::reset ()
{
  mCliReadIdx = 0;
  mCliWriteIdx = 0;
  mCliFd = -1;

  mSerReadIdx = 0;
  mSerWriteIdx = 0;

  mSerClosed = false;
  memset (mCliBuf, '\0', BUF_SIZE);
  memset (mSerBuf, '\0', BUF_SIZE);
}