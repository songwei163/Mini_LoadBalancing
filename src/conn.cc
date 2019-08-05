//
// Created by S on 2019/8/3.
//

#include "conn.h"
#include "clog.h"

void conn::initClt (int sockfd, const sockaddr_in &client_addr)
{
  mCliFd = sockfd;
  mCliAddr = client_addr;
}

void conn::initSrv (int sockfd, const sockaddr_in &server_addr)
{
  mSerFd = sockfd;
  mSerAddr = server_addr;
}

conn::conn ()
{
  mSerFd = -1;
  mCliBuf = new char[BUF_SIZE];
  mSerBuf = new char[BUF_SIZE];

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

RET_CODE conn::readCli ()
{
  int bytesRead = 0;
  while (true)
    {
      if (mCliReadIdx >= BUF_SIZE)
        {
          LOG_ERROR ("%s", "the client read buffer is full, let server write");
          return BUFFER_FULL;
        }

      bytesRead = recv (mCliFd, mCliBuf + mCliReadIdx, BUF_SIZE - mCliReadIdx, 0);
      if (bytesRead == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              break;
            }
          return IOERR;
        }
      else if (bytesRead == 0)
        {
          return CLOSED;
        }

      mCliReadIdx += bytesRead;
    }
  return ((mCliReadIdx - mCliWriteIdx) > 0) ? OK : NOTHING;
}

RET_CODE conn::readSer ()
{
  int bytesRead = 0;
  while (true)
    {
      if (mSerReadIdx >= BUF_SIZE)
        {
          LOG_ERROR ("%s", "the server read buffer is full, let client write");
          return BUFFER_FULL;
        }

      bytesRead = recv (mSerFd, mSerBuf + mSerReadIdx, BUF_SIZE - mSerReadIdx, 0);
      if (bytesRead == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              break;
            }
          return IOERR;
        }
      else if (bytesRead == 0)
        {
          LOG_ERROR ("%s", "the server should not close the persist connection");
          return CLOSED;
        }

      mSerReadIdx += bytesRead;
    }
  return ((mSerReadIdx - mSerWriteIdx) > 0) ? OK : NOTHING;
}

RET_CODE conn::writeCli ()
{
  int bytesWrite = 0;
  while (true)
    {
      if (mSerReadIdx <= mSerWriteIdx)
        {
          mSerReadIdx = 0;
          mSerWriteIdx = 0;
          return BUFFER_EMPTY;
        }

      bytesWrite = send (mCliFd, mSerBuf + mSerWriteIdx, mSerReadIdx - mSerWriteIdx, 0);
      if (bytesWrite == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              return TRY_AGAIN;
            }

          LOG_ERROR ("write client socket failed, %s", strerror (errno));
          return IOERR;
        }

      else if (bytesWrite == 0)
        {
          return CLOSED;
        }

      mSerWriteIdx += bytesWrite;
    }
}

RET_CODE conn::writeSer ()
{
  int bytesWrite = 0;
  while (true)
    {
      if (mCliReadIdx <= mCliReadIdx)
        {
          mCliReadIdx = 0;
          mCliReadIdx = 0;
          return BUFFER_EMPTY;
        }

      bytesWrite = send (mSerFd, mCliBuf + mCliWriteIdx, mCliReadIdx - mCliWriteIdx, 0);
      if (bytesWrite == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              return TRY_AGAIN;
            }

          LOG_ERROR ("write server socket failed, %s", strerror (errno));
          return IOERR;
        }
      else if (bytesWrite == 0)
        {
          return CLOSED;
        }

      mCliWriteIdx += bytesWrite;
    }
}