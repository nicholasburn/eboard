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

#ifndef EBOARD_HELP_H
#define EBOARD_HELP_H 1

#include "widgetproxy.h"
#include "notebook.h"

namespace Help {

  class AboutDialog : public ModalDialog {
  public:
    AboutDialog();
  };

  class KeysDialog : public ModalDialog {
  public:
    KeysDialog();
  };

  class DebugDialog : public ModalDialog {
  public:
    DebugDialog();

  private:
    char *grabOutput(const char *cmdline);
    char pipedata[1024];

    friend void debugdlg_writecons(GtkWidget *w, gpointer data);
  };

  void debugdlg_writecons(GtkWidget *w, gpointer data);

  class GettingStarted : public NonModalDialog {
  public:
    GettingStarted();
  private:
    void compose();
    GtkWidget *text;
  };
}


#endif
