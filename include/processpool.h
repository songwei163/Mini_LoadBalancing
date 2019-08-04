//
// Created by S on 2019/8/3.
//

#ifndef _PROCESSPOOL_H_
#define _PROCESSPOOL_H_

#include "common.h"
#include "fdwrapper.h"

class process {
 public:
  process () : mPid (-1)
  {}
 public:
  int mBusyRatio;
  pid_t mPid;
  int mPipeFd[2];
};

static int EPOLL_WAIT_TIME = 5000;
static int sigPipeFd[2];

static void sigHandler (int sig);

static void addSig (int sig, void(handler) (int), bool restart = true);

//               conn     host    mgr
template<typename C, typename H, typename M>
class processPool {
 public:
  ~processPool ()
  {
    delete[]mSubProcess;
  }
  void run (const vector<H> &arg);
  void runChild (const vector<H> &arg);
  void runParent ();

 public:
  static processPool<C, H, M> *create (int listenFd, int ProcessNum = 8)
  {
    if (mInstance == nullptr)
      {
        mInstance = new processPool<C, H, M> (listenFd, ProcessNum);
      }
    return mInstance;
  }
 private:
  void set_sig_pipe ();
 private:
  processPool (int listenFd, int ProcessNum = 8);
 private:
  static const int MAX_PROCESS_NUMBER = 16;
  static const int USER_PER_PROCESS = 65536;
  static const int MAX_EVENT_NUMBER = 10000;
  int mProcessNum;
  int mIdx;
  int mEpollFd;
  int mListenFd;
  int mStop;
  process *mSubProcess;
  static processPool<C, H, M> *mInstance;
};

template<typename C, typename H, typename M>
processPool<C, H, M> *processPool<C, H, M>::mInstance = nullptr;

template<typename C, typename H, typename M>
processPool<C, H, M>::processPool (int listenFd, int ProcessNum) :
    mListenFd (listenFd), mProcessNum (ProcessNum), mIdx (-1), mStop (false)
{
  assert((ProcessNum > 0) && (ProcessNum <= MAX_PROCESS_NUMBER));
  mSubProcess = new process[ProcessNum];
  assert (mSubProcess);

  for (int i = 0; i < ProcessNum; ++i)
    {
      int ret = socketpair (PF_UNIX, SOCK_STREAM, 0, mSubProcess[i].mPipeFd);
      assert (ret == 0);

      mSubProcess[i].mPid = fork ();
      assert (mSubProcess[i].mPid >= 0);
      if (mSubProcess[i].mPid > 0)
        {
          close (mSubProcess[i].mPipeFd[1]);
          mSubProcess[i].mBusyRatio = 0;
          continue;
        }
      else
        {
          close (mSubProcess[i].mPipeFd[0]);
          mIdx = i;
          break;
        }
    }
}

template<typename C, typename H, typename M>
void processPool<C, H, M>::run (const vector<H> &arg)
{
  if (mIdx != -1)
    {
      runChild (arg);
      return;
    }
  runParent ();
}

template<typename C, typename H, typename M>
void processPool<C, H, M>::runChild (const vector<H> &arg)
{
  //向epoll注册信号监听和创建管道
  set_sig_pipe ();

  //向epoll注册父进程监听
  int pipeFdRead = mSubProcess[mIdx].mPipeFd[1];
  add_read_fd (mEpollFd, pipeFdRead);

  //管理
  M *manager = new M (mEpollFd, arg[mIdx]);
  assert (manager);

  int number = 0;
  int ret = -1;
  struct epoll_event revents[MAX_EVENT_NUMBER];

  //统一事件源 处理各种连接
  while (!mStop)
    {
      number = epoll_wait (mEpollFd, revents, MAX_EVENT_NUMBER, EPOLL_WAIT_TIME);
      if ((number < 0) && (errno != EINTR))
        {
          cout << "epoll failure\n";
          break;
        }

      if (number == 0)
        {
          //manager
          continue;
        }

      for (int i = 0; i < number; ++i)
        {
          int sockfd = revents[i].data.fd;
          if ((sockfd == pipeFdRead) && (revents[i].events & EPOLLIN))
            {
              int client = 0;
              ret = recv (sockfd, (char *) &client, sizeof (client), 0);
              if (((ret < 0) && (errno != EAGAIN)) || ret == 0)
                {
                  continue;
                }

              else
                {
                  struct sockaddr_in clientAddress;
                  socklen_t clientAddrLen = sizeof (clientAddress);
                  int connfd = accept (mListenFd, (
                      struct sockaddr *) &clientAddress, &clientAddrLen);
                  if (connfd < 0)
                    {
                      LOG_ERROR ("errno: %s", strerror (errno));
                      continue;
                    }

                  add_read_fd (mEpollFd, connfd);
                  //
                }
            }

            //信号处理
          else if ((sockfd == sigPipeFd[0]) && (revents[i].events & EPOLLIN))
            {
              int sig;
              char signals[1024];
              ret = recv (sigPipeFd[0], signals, sizeof (signals), 0);
              if (ret <= 0)
                {
                  continue;
                }
              else
                {
                  for (int i = 0; i < ret; ++i)
                    {
                      switch (signals[i])
                        {
                          case SIGCHLD:
                            {
                              pid_t pid;
                              int stat;
                              while ((pid = waitpid (-1, &stat, WNOHANG)) > 0)
                                {
                                  continue;
                                }
                              break;
                            }
                          case SIGTERM:
                          case SIGINT:
                            {
                              mStop = true;
                              break;
                            }
                          default:
                            {
                              break;
                            }
                        }
                    }
                }
            }

            //数据处理
          else if (revents[i].events & EPOLLIN)
            {

            }

            //数据处理
          else if (revents[i].events & EPOLLOUT)
            {

            }
          else
            {
              continue;
            }

        }
    }
  //关闭epoll监听，释放资源
  close (pipeFdRead);
  close (mEpollFd);
}

template<typename C, typename H, typename M>
void processPool<C, H, M>::runParent ()
{
  cout << "parent process: OK" << endl;

  //向epoll注册信号监听和创建管道
  set_sig_pipe ();

  //向epoll注册子进程监听
  for (int i = 0; i < mProcessNum; ++i)
    {
      add_read_fd (mEpollFd, mSubProcess[i].mPipeFd[0]);
    }

  //向epoll注册listen监听
  add_read_fd (mEpollFd, mListenFd);

  //统一事件源 处理各种连接
  struct epoll_event revents[MAX_EVENT_NUMBER];
  int number = 0;

  while (!mStop)
    {
      number = epoll_wait (mEpollFd, revents, MAX_EVENT_NUMBER, EPOLL_WAIT_TIME);
      if ((number < 0) && (errno != EINTR))
        {
          cout << "epoll failure\n";
          break;
        }

      if (number == 0)
        {
          cout << "epoll time out\n";
          continue;
        }

      for (int i = 0; i < number; ++i)
        {
          //客户连接
          int sockfd = revents[i].data.fd;
          if (sockfd == mListenFd)
            {
              //挑选逻辑服务器
            }

            //信号处理
          else if ((sockfd == sigPipeFd[0]) && (revents[i].events & EPOLLIN))
            {
              int sig;
              char signals[1024];

              int ret = recv (sigPipeFd[0], signals, sizeof (signals), 0);
              if (ret <= 0)
                {
                  continue;
                }
              else
                {
                  for (int i = 0; i < ret; ++i)
                    {
                      switch (signals[i])
                        {
                          //子进程退出
                          case SIGCHLD:
                            {
                              pid_t pid;
                              int stat;
                              while ((pid = waitpid (-1, &stat, WNOHANG)) > 0)
                                {
                                  for (int i = 0; i < mProcessNum; ++i)
                                    {
                                      LOG_NOTICE ("child %d join", i);
                                      close (mSubProcess[i].mPipeFd[0]);
                                      mSubProcess[i].mPid = -1;
                                    }
                                }

                              mStop = true;
                              for (int i = 0; i < mProcessNum; ++i)
                                {
                                  if (mSubProcess[i].mPid != -1)
                                    {
                                      mStop = false;
                                    }
                                }
                              break;
                            }

                          case SIGTERM:
                          case SIGINT:
                            {
                              LOG_NOTICE ("%s", "kill all the clild now");
                              for (int i = 0; i < mProcessNum; ++i)
                                {
                                  int pid = mSubProcess[i].mPid;
                                  if (pid != -1)
                                    {
                                      kill (pid, SIGTERM);
                                    }
                                }
                              break;
                            }

                          default:
                            {
                              break;
                            }
                        }
                    }
                }
            }

            //I/O处理
          else if (revents[i].events & EPOLLIN)
            {
              int busy_ratio = 0;
              int ret = recv (sockfd, (char *) &busy_ratio, sizeof (busy_ratio), 0);

              if (((ret < 0) && (errno != EAGAIN)) || ret == 0)
                {
                  continue;
                }

              for (int i = 0; i < mProcessNum; ++i)
                {
                  if (sockfd == mSubProcess[i].mPipeFd[0])
                    {
                      mSubProcess[i].mBusyRatio = busy_ratio;
                      break;
                    }
                }
              continue;
            }
        }
    }

  //关闭epoll监听，释放资源
  for (int i = 0; i < mProcessNum; ++i)
    {
      closefd (mEpollFd, mSubProcess[i].mPipeFd[0]);
    }
  close (mEpollFd);
}

template<typename C, typename H, typename M>
void processPool<C, H, M>::set_sig_pipe ()
{
  mEpollFd = epoll_create (5);
  assert (mEpollFd != -1);

  int ret = socketpair (PF_UNIX, SOCK_STREAM, 0, sigPipeFd);
  assert (ret != -1);

  setnonblocking (sigPipeFd[1]);
  add_read_fd (mEpollFd, sigPipeFd[0]);

  addSig (SIGCHLD, sigHandler);
  addSig (SIGTERM, sigHandler);
  addSig (SIGINT, sigHandler);
  addSig (SIGPIPE, SIG_IGN);
}

static void sigHandler (int sig)
{
  int saveErrno = errno;
  int msg = sig;
  send (sigPipeFd[1], (char *) &msg, 1, 0);
  errno = saveErrno;
}

static void addSig (int sig, void(handler) (int), bool restart)
{
  struct sigaction sa;
  memset (&sa, '\0', sizeof (sa));
  sa.sa_handler = handler;
  if (restart)
    {
      sa.sa_flags |= SA_RESTART;
    }

  sigfillset (&sa.sa_mask);
  assert (sigaction (sig, &sa, nullptr) != -1);
}

#endif //_PROCESSPOOL_H_