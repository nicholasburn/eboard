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

#ifndef TSTRING_H
#define TSTRING_H

#include <string>

// string tokenizer

class tstring {
 public:

  /* creates a new string tokenizer, with empty string, chomp false
     and fail value 0 */
  tstring();

  /* sets chomp flag. when true, the character that caused the token
     to end is not considered a candidate for the next token */
  void setChomp(bool v);

  /* sets the value tokenvalue returns when there are no more tokens */
  void setFail(int v);

  /* sets the working string and resets position to its start */
  void set(const std::string &s);

  /* keeps working string but resets position to the start */
  void reset();

  /* returns a pointer to the next token, t is string of delimiter
     characters */
  std::string &token(const std::string &sep);

  /* returns the integer value of the next token, delimited by
     characters of t and assumed to be in the given base */
  int tokenvalue(const std::string &sep, int base=10);

  /* returns false if number is zero, true if not, defval if no
     more tokens */
  bool tokenbool(const std::string &sep, bool defval);

  /* returns current token */
  std::string &curToken();

  /* advances to next token using last separator string */
  std::string &next();

  /* true if end of input string was reached */
  bool done() const;
  
 private:
  std::string ptoken, src, lastsep;
  size_t pos;
  bool   chomp;
  int    fail;
};

// file name manipulation
class file {
 public:
  static std::string basename(const std::string &path);

};

#endif
