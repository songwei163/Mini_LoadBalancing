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

static int EPOLL_WAIT_TIME = 5000;
static int sig_pipefd[2];

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
  int pipeReadFd = mSubProcess[mIdx].mPipeFd[1];
  add_read_fd (mEpollFd, pipeReadFd);

  epoll_event events[MAX_EVENT_NUMBER];

  M *mangaer = new M (mEpollFd, arg[mIdx]);
  assert (mangaer);

  int number = 0;
  int ret = -1;

  while (!mStop)
    {
      number = epoll_wait (mEpollFd, events, MAX_EVENT_NUMBER, EPOLL_WAIT_TIME);
      if ((number < 0) && (errno != EINTR))
        {
          LOG_ERROR ("%s", "epoll failure");
          break;
        }

      if (number == 0)
        {
            mangaer->recycle_conns();
        }

    }
}

template<typename C, typename H, typename M>
void processPool<C, H, M>::runParent ()
{
  cout << "parent process: OK" << endl;

}
#endif //_PROCESSPOOL_H_
