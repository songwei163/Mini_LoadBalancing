//
// Created by S on 2019/8/2.
//

#ifndef _MGR_H_
#define _MGR_H_

#include "common.h"

class host {
 public:
  char m_hostname[HOST_NAME_SIZE];
  int m_port;
  int m_connect;
};

#endif //_MGR_H_
