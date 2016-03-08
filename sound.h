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

#ifndef EBOARD_SOUND_H
#define EBOARD_SOUND_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include "tstring.h"
#include "widgetproxy.h"
#include "stl.h"

typedef enum {
  INT_WAVE,
  EXT_WAVE,
  EXT_PROGRAM,
  PLAIN_BEEP
} SoundEventType;

class SoundEventChangeListener {
 public:
  virtual void SoundEventChanged()=0;
};

class MultiBeep {
 public:
  MultiBeep(int _samplerate=44100, int _duration=100, int _pitch = 440, int _count = 1);
  ~MultiBeep();

  int SampleRate;
  int Duration;
  int Pitch;
  int Count;

  short int *data;
  int samples;
};

class SoundEvent {
 public:
  SoundEvent();

  SoundEvent operator=(SoundEvent &se);
  int operator==(SoundEvent &se);
  int operator!=(SoundEvent &se);

  void read(tstring &rcline);

  void play();
  void playHere();
  void edit(SoundEventChangeListener *listener);

  char *getDescription();

  SoundEventType type;
  int  Duration;
  int  Pitch;
  int  Count;
  char ExtraData[256];
  bool enabled;

 private:
  void sine_beep();

  void gstPlay(const string &_input);
  void gstBeep();

  char pvt[128];
};

ostream & operator<<(ostream &s,  SoundEvent e);

class SoundEventDialog : public ModalDialog {
 public:
  SoundEventDialog(SoundEvent *src, SoundEventChangeListener *listener);
 private:
  SoundEvent *obj;
  SoundEventChangeListener *hearer;
  GtkWidget *en[4], *rd[4], *fdlg, *tme;
  vector<GtkWidget *> sthemes;

  void apply(SoundEvent *dest);

  friend void snddlg_ok(GtkWidget *w,gpointer data);
  friend void snddlg_test(GtkWidget *w,gpointer data);
  friend void snddlg_browse(GtkWidget *w,gpointer data);
  friend void snddlg_picktheme(GtkMenuItem *w,gpointer data);
};

void snddlg_ok(GtkWidget *w,gpointer data);
void snddlg_test(GtkWidget *w,gpointer data);
void snddlg_browse(GtkWidget *w,gpointer data);
void snddlg_picktheme(GtkMenuItem *w,gpointer data);

#endif

