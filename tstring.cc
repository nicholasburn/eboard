/*

    eboard - chess client
    http://www.bergo.eng.br/eboard
    https://github.com/fbergo/eboard
    Copyright (C) 2000-2016 Felipe Bergo
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

#include <stdlib.h>
#include <string.h>
#include "tstring.h"

tstring::tstring() {
  chomp=false;
  fail=0;
  pos=0;
}

void tstring::set(const std::string &s) {
  pos=0;
  src=s;
  ptoken.clear();
}

std::string &tstring::token(const std::string &sep) {
  if (sep != lastsep) lastsep = sep;
  auto j=src.size();

  ptoken.clear();
  if (pos>=j) return ptoken;
  
  // skip to first position of token
  while ( sep.find_first_of(src[pos]) != std::string::npos ) {
    pos++;
    if (pos>=j)return ptoken;
  }

  while ( sep.find_first_of(src[pos]) == std::string::npos ) {
    ptoken += src[pos];
    pos++;
    if (pos>=j) break;
  }

  if ( chomp && pos<j ) ++pos;

  //printf("src=[%s] pos=%d token=[%s]\n",src.c_str(),(int)pos,ptoken.c_str());
  return(ptoken);
}

std::string & tstring::next() {
  auto j=src.size();

  ptoken.clear();  
  if (pos>=j) return ptoken;
  
  // skip to first position of token
  while ( lastsep.find_first_of(src[pos]) != std::string::npos ) {
    pos++;
    if (pos>=j) return ptoken;
  }

  while ( lastsep.find_first_of(src[pos]) == std::string::npos ) {
    ptoken += src[pos];
    pos++;
    if (pos>=j) break;
  }

  if ( chomp && pos<j ) ++pos;

  return(ptoken);
}

bool tstring::done() const {
  return(pos>=src.size());
}

int tstring::tokenvalue(const std::string &sep, int base) {
  if (sep != lastsep) lastsep = sep;
  std::string v=token(sep);
  if (v.empty()) return fail;
  return(std::stoi(v,0,base));
}

bool tstring::tokenbool(const std::string &sep, bool defval) {
  if (sep != lastsep) lastsep = sep;
  std::string v=token(sep);
  if (v.empty()) return defval;
  return(std::stoi(v,0)!=0);
}

void tstring::setChomp(bool v) {
  chomp=v;
}

void tstring::setFail(int v) {
  fail=v;
}

void tstring::reset() {
  pos=0;
  ptoken.clear();
}

std::string & tstring::curToken() {
  return(ptoken);
}

std::string file::basename(const std::string &path) {
  auto p = path.find_last_of('/');
  if (p == std::string::npos) p=0; else ++p;
  auto q = path.size()-1;
  while(path[q] < 32 && q>0) --q;
  std::string out = path.substr(p,q-p+1);
  return out;
}
