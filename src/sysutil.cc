//
// Created by S on 2019/8/2.
//

#include "sysutil.h"
#include "tinystr.h"
#include "tinyxml.h"
#include <string>

const char *version = "1.0";

/**
 *
 * @param exe
 */
void Usage (char *exe)
{
  cout << "Usage:  " << basename (exe) << "[-x] [-v] [-h] [-f config]" << endl;
}

/**
 *
 * @param version
 */
void Version (const char *version)
{
  cout << "loadBalance: version " << version << endl;
}

/**
 *
 * @param argc
 * @param argv
 */
void ParseCmdArg (int argc, char *argv[], char *cfgFlie)
{
  int option;
  const char *optstring = "f:xvh";
  while ((option = getopt (argc, argv, optstring)) != -1)
    {
      switch (option)
        {
          case 'f':
            strcpy (cfgFlie, optarg);
          break;
          case 'x':
            break;
          case 'v':
            Version (version);
          break;
          case 'h':
            Usage (argv[0]);
          break;
          case '?':
            break;
        }
    }
}

int LoadCfgFile (const char *fileName, char *buf)
{
  if (*fileName == '\0')
    {
      return -1;
    }

  int cfgFd;
  if ((cfgFd = open (fileName, O_RDONLY)) < 0)
    {
      return -1;
    }

  struct stat statBuf;
  if (fstat (cfgFd, &statBuf) == -1)
    {
      return -1;
    }

  size_t fileLen = statBuf.st_size;
  buf = new char[fileLen + 1];

  if (read (cfgFd, buf, fileLen) < 0)
    {
      return -1;
    }
  close (cfgFd);
  return true;
}

int LoadCfgFile (const char *filename, char *&buf)
{
  if (*filename == '\0')
    return -1;
  int fd;
  if ((fd = open (filename, O_RDONLY)) < 0)
    return -1;

  struct stat statbuf;
  if (fstat (fd, &statbuf) < 0)
    return -1;

  size_t file_len = statbuf.st_size;
  buf = new char[file_len + 1];

  if (read (fd, buf, file_len) < 0)
    return -1;

  close (fd);
  return 0;
}

int ParseCfgFile (char *filename, vector<host> &balance_srv, vector<host> &logical_srv)
{
  TiXmlDocument doc;
  if (doc.LoadFile (filename))
    {
      doc.Print ();
    }
  else
    {
      cout << "can not open " << basename (filename);
    }

  TiXmlElement *Host = doc.RootElement ();
  TiXmlElement *Balance = Host->FirstChildElement ();

  TiXmlElement *balanceHost = Balance->FirstChildElement ();
  TiXmlElement *hostElement = balanceHost->FirstChildElement ();

  host tmp;
  for (; hostElement != nullptr; hostElement = hostElement->NextSiblingElement ())
    {
      //cout << hostElement->Value () << " : " << hostElement->GetText () << endl;
      if (strcmp (hostElement->Value (), "ip") == 0)
        {
          strcpy (tmp.m_hostname, hostElement->GetText ());
        }
      else if (strcmp (hostElement->Value (), "port") == 0)
        {
          tmp.m_port = atoi (hostElement->GetText ());
        }
      else if (strcmp (hostElement->Value (), "connect") == 0)
        {
          tmp.m_connect = atoi (hostElement->GetText ());
        }
    }

  balance_srv.push_back (tmp);

  TiXmlElement *logical = Balance->NextSiblingElement ();
  TiXmlElement *logical_host = logical->FirstChildElement ();
  for (; logical_host != nullptr; logical_host = logical_host->NextSiblingElement ())
    {
      hostElement = logical_host->FirstChildElement ();
      for (; hostElement != nullptr; hostElement = hostElement->NextSiblingElement ())
        {
          //cout << hostElement->Value () << " : " << hostElement->GetText () << endl;
          if (strcmp (hostElement->Value (), "ip") == 0)
            {
              strcpy (tmp.m_hostname, hostElement->GetText ());
            }
          else if (strcmp (hostElement->Value (), "port") == 0)
            {
              tmp.m_port = atoi (hostElement->GetText ());
            }
          else if (strcmp (hostElement->Value (), "connect") == 0)
            {
              tmp.m_connect = atoi (hostElement->GetText ());
            }

        }
      logical_srv.push_back (tmp);
    }
  return 0;
}
