//
// Created by S on 2019/8/3.
//

#ifndef _PROCESSPOOL_H_
#define _PROCESSPOOL_H_

#include "common.h"
#include "fdwrapper.h"

class process {
 public:
  process () : m_pid (-1)
  {}
 public:
  int m_busy_ratio;
  pid_t m_pid;
  int m_pipefd[2];
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

template<typename C, typename H, typename M>
processPool<C, H, M>::processPool (int listenFd, int ProcessNum) :
    mListenFd (listenFd), mProcessNum (ProcessNum), mIdx (-1), mStop (false)
{
  assert((ProcessNum > 0) && (ProcessNum <= MAX_PROCESS_NUMBER));
  mSubProcess = new process[ProcessNum];
  assert (mSubProcess);

  for (int i = 0; i < ProcessNum; ++i)
    {
      int ret = sockpair (PF_UNIX, SOCK_STREAM, 0, mSubProcess[i].m_pipefd);
      assert (ret == 0);

      mSubProcess[i].m_pid = fork ();
      assert (mSubProcess[i].m_pid >= 0);
      if (mSubProcess[i].m_pid > 0)
        {
          close (mSubProcess[i].m_pipefd[1]);
          mSubProcess[i].m_busy_ratio = 0;
          continue;
        }
      else
        {
          close (mSubProcess[i].m_pipefd[0]);
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
  cout << "child process: OK" << endl;
}

template<typename C, typename H, typename M>
void processPool<C, H, M>::runParent ()
{
  cout << "parent process: OK" << endl;

}
#endif //_PROCESSPOOL_H_
