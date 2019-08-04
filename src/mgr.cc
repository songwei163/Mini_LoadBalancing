//
// Created by S on 2019/8/2.
//

#include "mgr.h"
#include "common.h"
#include "clog.h"

int mgr::mEpollFd = -1;

mgr::mgr (int epollFd, const host &srv) : m_logic_srv (srv)
{
  mEpollFd = epollFd;
  struct sockaddr_in address;
  memset (&address, 0, sizeof (address));
  address.sin_family = AF_INET;
  inet_pton (AF_INET, srv.mHostName, &address.sin_addr);
  address.sin_port = htons (srv.mPort);
  //LOG_ERROR ("logcial srv host info: (%s, %d)", srv.mHostName, srv.mPort);
  printf ("logcial srv host info: (%s, %d)", srv.mHostName, srv.mPort);
}

mgr::~mgr ()
{

}
