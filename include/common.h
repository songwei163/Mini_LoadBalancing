//
// Created by S on 2019/8/2.
//

#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
//#include <bits/sigaction.h>
#include <csignal>
#include <wait.h>


#include <fcntl.h>
#include <libgen.h>

#define ERR_EXIT(m) \
    do{\
    printf(m);\
    exit(EXIT_FAILURE);\
    }while(0)

#define FILE_NAME_MAX_SIZE 128
#define HOST_NAME_SIZE 64

#endif //_COMMON_H_
