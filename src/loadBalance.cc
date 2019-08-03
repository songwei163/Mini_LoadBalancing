//
// Created by S on 2019/8/2.
//

#include "sysutil.h"
#include "clog.h"
#include "conn.h"
#include "processpool.h"

int main (int argc, char *argv[])
{
  if (getuid () != 0)
    {
      cout << "loadBalance must be started as root" << endl;
      exit (EXIT_FAILURE);
    }

  //log_init (LL_DEBUG, "loadBalanceLog", "./log");

  if (argc <= 1)
    {
      LOG_ERROR ("cmd args too less, please help");
      exit (EXIT_FAILURE);
    }

  char configFile[FILE_NAME_MAX_SIZE];
  memset (configFile, 0, FILE_NAME_MAX_SIZE);

  ParseCmdArg (argc, argv, configFile);

  //cout << "configFile : " << configFile << endl;

  vector<host> balance_srv;
  vector<host> logical_srv;
  //listen host : ip port          balance_host
  //logical host: ip port conns    logical_host
  //ParseCfgFile();
  if (ParseCfgFile (configFile, balance_srv, logical_srv) < 0)
    ERR_EXIT("ParseCfgFile");

  //for (int i = 0; i != logical_srv.size (); ++i)
  // {
  // cout << logical_srv[i].m_hostname << endl;
  //}

/***************************************************************************************************************************/

  int listenFd = TcpServer (balance_srv[0].mHostName, balance_srv[0].mPort);
  processPool<conn, host, mgr> *pool = processPool<conn, host, mgr>::create (listenFd, logical_srv.size ());

  if (pool)
    {
      cout << "a" << endl;
    }
  return 0;

}