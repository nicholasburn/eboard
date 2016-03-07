/* $Id: network.h,v 1.23 2010/06/29 18:04:37 bergo Exp $ */

/*

    eboard - chess client
    http://eboard.sourceforge.net
    Copyright (C) 2000-2010 Felipe Paulo Guazzi Bergo
    fbergo/at/gmail/dot/com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/



#ifndef EBOARD_NETWORK_H
#define EBOARD_NETWORK_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "config.h"

#ifdef NEED_TCP_H
#include <netinet/tcp.h>
#endif

#include <arpa/inet.h>

#include <gtk/gtk.h>

#include "stl.h"
#include "eboard.h"

class PidIssuer {
 public:
  virtual void farewellPid(int dpid)=0;
};

class Parent {
 public:
  Parent(PidIssuer *a,int b);
  int operator==(int v);
  PidIssuer *issuer;
  int pid;
};

class PidRing : public SigChildHandler {
 public:
  virtual ~PidRing() {}
  void add(PidIssuer *po, int pid);
  void remove(PidIssuer *po);
  void funeral(int pid);
  virtual void ZombieNotification(int pid);
 private:
  list<Parent *> parents;
};

class NetConnection {
 public:
  NetConnection();
  virtual ~NetConnection();

  virtual int isConnected();

  // 0 ok, -1 error, msg in getError
  virtual int  open()=0;

  virtual void close()=0;
  
  // -1 if nothing available, 0 when line ok
  virtual int readLine(char *tbuffer,int limit)=0;

  virtual void writeLine(const char *tbuffer)=0;

  virtual int  readPartial(char *tbuffer,int limit)=0;
  virtual bool bufferMatch(const char *match)=0;

  virtual char *getError()=0;

  virtual int  hasTimeGuard();

  virtual int  getReadHandle()=0;

  char HostName[128];
  char HostAddress[96];

  int  TimeGuard;

  virtual void notifyReadReady(IONotificationInterface *target);
  virtual void sendReadNotify();

  void cleanUp();

 private:
  int  TagRead;
  IONotificationInterface *listener;

  friend void netconn_read_notify(gpointer data, gint source, 
				  GdkInputCondition cond);
};

void netconn_read_notify(gpointer data, gint source, 
			 GdkInputCondition cond);

class RingBuffer {
 public:
  RingBuffer(int _size);
  ~RingBuffer();

  bool empty() const;
  bool full() const;
  
  char pop();
  bool push(char c);
  char last() const;

  string getLine();
  
  string toString(int maxlen=-1) const;

  void status();
  
 private:
  char *data;
  int rp,wp,len,size;
};

class BufferedConnection : public NetConnection {
 public:
 BufferedConnection() : buffer(64<<10) { brokenpeer = false; }
  
  virtual int  readPartial(char *tbuffer,int limit);
  virtual bool bufferMatch(const char *match);
 protected:
  virtual int innerReadLine(char *tbuffer,int limit,int handle);
  int  consume(int handle, int amount=128);
  int  produce(char *tbuffer,int limit,int handle);
  RingBuffer buffer;
  bool brokenpeer;
};

class DirectConnection : public BufferedConnection {
 public:
  DirectConnection(const char *hostname,int port);

  int open();
  void close();
  int readLine(char *tbuffer,int limit);
  void writeLine(const char *obuffer);
  int isConnected();

  char *getError();
  int  getReadHandle();

 private:
  int Port;
  int Connected;
  char errorMessage[128];

  struct hostent *he;
  struct sockaddr_in sa;
  int netsocket;
};

class IncomingConnection : public BufferedConnection {
 public:
  IncomingConnection(int port);

  int open();
  void close();
  int readLine(char *tbuffer, int limit);
  void writeLine(const char *obuffer);
  int isConnected();

  char *getError();
  int  getReadHandle();

 private:
  int Port;
  int Connected;
  char errorMessage[128];
  int netsocket;

  int createSocket();
  int acceptConnection();
};

class PipeConnection : public BufferedConnection,
                       public PidIssuer                       
{
 public:
  PipeConnection(int _pin,int _pout);
  /// chess engine constructor
  PipeConnection(const char *helperbin,const char *arg1=NULL,const char *arg2=NULL,const char *arg3=NULL,const char *arg4=NULL);
  /// timeseal constructor
  PipeConnection(const char *host,int port, const char *helperbin,const char *helpersuffix);
  virtual ~PipeConnection();

  int Quiet;

  int isConnected();

  // 0 ok, -1 error, msg in getError
  int  open();

  void close();

  void setHandshake(const char *s);
  
  // -1 if nothing available, 0 when line ok
  int readLine(char *tbuffer,int limit);
  void writeLine(const char *obuffer);
  char *getError();
  int   getReadHandle();

  virtual void farewellPid(int dpid);
  void setMaxWaitTime(double msecs);

 private:
  void init();
  void checkChildren();
  friend gboolean sched_close(gpointer data);

  int  opmode; // 0=engine with bare args, 1=timeseal with network host
  int  pout, pin;
  int  Connected;
  int  Port;
  char HelperBin[512];
  char errorMessage[128];
  vector<const char *> args;
  int  pid;
  int  toid; // timeout
  string handshake;
  double MaxWaitTime; // msecs
};

gboolean sched_close(gpointer data);

class FallBackConnection : public NetConnection {
 public:
  FallBackConnection();
  ~FallBackConnection();
  void append(NetConnection *nc);

  virtual int  isConnected();
  virtual int  open();
  virtual void close();
  virtual int  readLine(char *tbuffer,int limit);
  virtual void writeLine(const char *tbuffer);
  virtual int  readPartial(char *tbuffer,int limit);
  virtual bool bufferMatch(const char *match);
  virtual char *getError();
  virtual int getReadHandle();

  virtual void notifyReadReady(IONotificationInterface *target);
  virtual void sendReadNotify();
 private:
  int Connected;
  list<NetConnection *>::iterator current;
  list<NetConnection *> candidates;
  char errorMessage[128];
};

#endif
