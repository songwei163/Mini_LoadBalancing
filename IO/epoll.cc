//
// Created by s on 19-7-29.
//

#include "common.h"
#include <sys/epoll.h>

void add_event (int epfd, int fd, int event)
{
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = event;
  epoll_ctl (epfd, EPOLL_CTL_ADD, fd, &ev);
}

void mod_event (int epfd, int fd, int event)
{
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = event;
  epoll_ctl (epfd, EPOLL_CTL_MOD, fd, &ev);
}

void del_event (int epfd, int fd, int event)
{
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = event;
  epoll_ctl (epfd, EPOLL_CTL_DEL, fd, &ev);
}

int main (int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "usage: ./epoll ip port\n");
      return -1;
    }

  int sockfd;
  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      fprintf (stderr, "socket failed\n");
      return -1;
    }

  struct sockaddr_in addrSer;
  memset (&addrSer, 0, sizeof (addrSer));
  addrSer.sin_family = AF_INET;
  addrSer.sin_port = htons ((atoi (argv[2])));
  inet_pton (AF_INET, argv[1], &addrSer.sin_addr);

  int opt = 1;
  if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof (opt)))
    {
      fprintf (stderr, "setsockopt failed\n");
      return -1;
    }

  socklen_t addrlen = sizeof (addrSer);
  if (bind (sockfd, (struct sockaddr *) &addrSer, addrlen) < 0)
    {
      fprintf (stderr, "bind failed\n");
    }

  if (listen (sockfd, 5) == -1)
    {
      fprintf (stderr, "listen failed\n");
      return -1;
    }

  /////////////////////////////////////////////////////////////////////////////////////////////
  int client_count = 0;
  int epoll_fd = epoll_create (1024);
  struct epoll_event client_fds[256 + 1];
  memset (client_fds, 0, sizeof (epoll_event));

  add_event (epoll_fd, sockfd, EPOLLIN);
  /////////////////////////////////////////////////////////////////////////////////////////////

  char buf[256] = "";
  int i;
  while (1)
    {
      int ret = epoll_wait (epoll_fd, client_fds, 1024, -1);
      if (ret == -1)
        {
          fprintf (stderr, "epoll failed\n");
          return -1;
        }
      else if (ret == 0)
        {
          printf ("time out\n");
          continue;
        }

      else
        {
          for (i = 0; i < ret; ++i)
            {
              if ((client_fds[i].data.fd == sockfd) && (client_fds[i].events == EPOLLIN))
                {
                  struct sockaddr_in addrCli;
                  int connfd;
                  socklen_t addrlen = sizeof (addrCli);
                  connfd = accept (sockfd, (struct sockaddr *) &addrCli, &addrlen);

                  if (connfd == -1)
                    {
                      fprintf (stderr, "accept failed\n");
                      break;
                    }
                  add_event (epoll_fd, connfd, EPOLLIN);
                }

              else if (client_fds[i].events == EPOLLIN)
                {
                  recv (client_fds->data.fd, buf, 256, 0);
                  send (client_fds->data.fd, buf, strlen (buf) + 1, 0);
                }
            }
        }

    }
}
