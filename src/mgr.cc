//
// Created by S on 2019/8/2.
//

#include "mgr.h"
#include "common.h"
#include "clog.h"

int mgr::mEpollFd = -1;

int mgr::conn2srv (const sockaddr_in &address)
{
  int sockfd = socket (PF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      return -1;
    }

  if (connect (sockfd, (struct sockaddr *) &address, sizeof (address)) != 0)
    {
      close (sockfd);
      return -1;
    }
  return sockfd;
}

mgr::mgr (int epollFd, const host &srv) : m_logic_srv (srv)
{
  mEpollFd = epollFd;
  struct sockaddr_in address;
  memset (&address, 0, sizeof (address));
  address.sin_family = AF_INET;
  inet_pton (AF_INET, srv.mHostName, &address.sin_addr);
  address.sin_port = htons (srv.mPort);
  //LOG_ERROR ("logcial srv host info: (%s, %d)", srv.mHostName, srv.mPort);
  printf ("logcial srv host info: (%s, %d)\n", srv.mHostName, srv.mPort);

  for (int i = 0; i < srv.mConnect; ++i)
    {
      sleep (1);
      int sockfd = conn2srv (address);
      if (sockfd < 0)
        {
          LOG_ERROR ("build connection %d failed", i);
        }

      else
        {
          printf ("build connection %d to server success\n", i);
          //LOG_NOTICE ("build connection %d to server success", i);
          conn *tmp = nullptr;
          try
            {
              tmp = new conn;
            }
          catch (...)
            {
              close (sockfd);
              continue;
            }
          tmp->initSrv (sockfd, address);
          mCons.insert (pair<int, conn *> (sockfd, tmp));
        }
    }
}

mgr::~mgr ()
{

}

conn *mgr::pickConn (int cliFd)
{
  if (mCons.empty ())
    {
      printf ("%s\n", "not enough srv connections to server");
      return nullptr;
    }

  map<int, conn *>::iterator iter = mCons.begin ();
  int serFd = iter->first;
  conn *tmp = iter->second;

  if (tmp == nullptr)
    {
      printf ("%s\n", "empty server connection object");
      return nullptr;
    }

  mCons.erase (iter);
  mUsed.insert (pair<int, conn *> (cliFd, tmp));
  mUsed.insert (pair<int, conn *> (serFd, tmp));

  add_read_fd (mEpollFd, cliFd);
  add_read_fd (mEpollFd, serFd);
  return tmp;
}

int mgr::getUsedConnCnt ()
{
  return mUsed.size ();
}

RET_CODE mgr::process (int fd, OP_TYPE type)
{
  conn *connection = mUsed[fd];
  if (connection == nullptr)
    {
      return NOTHING;
    }

  if (connection->mCliFd == fd)
    {
      int srvfd = connection->mSerFd;
      switch (type)
        {
          case READ:
            {
              RET_CODE res = connection->readCli ();
              switch (res)
                {
                  case OK:
                    {
                      LOG_DEBUG ("content read from client: %s", connection->mCliBuf);
                    }

                  case BUFFER_FULL:
                    {
                      modfd (mEpollFd, srvfd, EPOLLOUT);
                      break;
                    }

                  case IOERR:
                  case CLOSED:
                    {
                      freeConn (connection);
                      return CLOSED;
                    }

                  default:
                    break;
                }

              if (connection->mSerClosed)
                {
                  freeConn (connection);
                  return CLOSED;
                }
              break;
            }

          case WRITE:
            {
              RET_CODE res = connection->writeCli ();
              switch (res)
                {
                  case TRY_AGAIN:
                    {
                      modfd (mEpollFd, fd, EPOLLOUT);
                      break;
                    }

                  case BUFFER_EMPTY:
                    {
                      modfd (mEpollFd, srvfd, EPOLLIN);
                      modfd (mEpollFd, fd, EPOLLIN);
                      break;
                    }

                  case IOERR:
                  case CLOSED:
                    {
                      freeConn (connection);
                      return CLOSED;
                    }

                  default:
                    break;
                }

              if (connection->mSerClosed)
                {
                  freeConn (connection);
                  return CLOSED;
                }
              break;
            }

          default:
            {
              LOG_ERROR ("%s", "other operation not support yet");
              break;
            }
        }
    }

  if (connection->mSerFd == fd)
    {
      int cltfd = connection->mCliFd;
      switch (type)
        {
          case READ:
            {
              RET_CODE res = connection->readSer ();
              switch (res)
                {
                  case OK:
                    {
                      LOG_DEBUG ("content read from server: %s", connection->mSerBuf);
                    }

                  case BUFFER_FULL:
                    {
                      modfd (mEpollFd, cltfd, EPOLLOUT);
                      break;
                    }

                  case IOERR:
                  case CLOSED:
                    {
                      modfd (mEpollFd, cltfd, EPOLLOUT);
                      connection->mSerClosed = true;
                      break;
                    }

                  default:
                    break;
                }
              break;
            }

          case WRITE:
            {
              RET_CODE res = connection->writeSer ();
              switch (res)
                {
                  case TRY_AGAIN:
                    {
                      modfd (mEpollFd, fd, EPOLLOUT);
                      break;
                    }

                  case BUFFER_EMPTY:
                    {
                      modfd (mEpollFd, cltfd, EPOLLIN);
                      modfd (mEpollFd, fd, EPOLLIN);
                      break;
                    }

                  case IOERR:
                  case CLOSED:
                    {
                      modfd (mEpollFd, cltfd, EPOLLOUT);
                      connection->mSerClosed = true;
                      break;
                    }

                  default:
                    {
                      break;
                    }
                }
              break;
            }

          default:
            {
              LOG_ERROR ("%s", "other operation not support yet");
              break;
            }
        }
    }

  else
    {
      return NOTHING;
    }

  return OK;
}

void mgr::recycleConns ()
{

}

void mgr::freeConn (conn *connection)
{
  int cltfd = connection->mCliFd;
  int srvfd = connection->mSerFd;
  closefd (mEpollFd, cltfd);
  closefd (mEpollFd, srvfd);
  mUsed.erase (cltfd);
  mUsed.erase (srvfd);
  connection->reset ();
  mFreed.insert (pair<int, conn *> (srvfd, connection));
}
