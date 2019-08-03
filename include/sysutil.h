//
// Created by S on 2019/8/2.
//

#ifndef _SYSUTIL_H_
#define _SYSUTIL_H_

#include "common.h"
#include "mgr.h"

void Usage (char *exe);
void Version (const char *version);
void ParseCmdArg (int argc, char *argv[], char *cfgFlie);
int LoadCfgFile (const char *filename, char *&buf);
int ParseCfgFile (char *filename, vector<host> &balance_srv, vector<host> &logical_srv);
int TcpServer (const char *ip, short port);



#endif //_SYSUTIL_H_
