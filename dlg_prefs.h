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

#ifndef EBOARD_PREFS
#define EBOARD_PREFS

#include "eboard.h"
#include "sound.h"
#include "widgetproxy.h"

class PreferencesDialog : public ModalDialog,
                          public SoundEventChangeListener,
                          public JoystickListener
{
 public:
  PreferencesDialog();
  virtual ~PreferencesDialog();

  void SoundEventChanged();

  virtual void joystickEvent(JoystickEventType jet, int number, int value);

 private:
  GtkWidget *tabposb[4],*efont[NFONTS],*fontdlg,*plainb,
    *showratb, *autologb, *showts, *sbacke, *special[4], /* *allob[2], */ 
    *asp, *aso, *afn, *sgb, *hsb, *chsb, *coct, *dhsb, *aqbar, *lowtime,
    *msecth, *msecdig, *wget, *smoothb, *xfont[NFONTS], 
    *sndon[N_SOUND_EVENTS],*sndtest[N_SOUND_EVENTS],*sndedit[N_SOUND_EVENTS],
    *sndd[N_SOUND_EVENTS], *jcl, *jctl, *jmode, *jspeed;
  BoxedLabel *fm[NFONTS];
  SoundEvent sndcopy[N_SOUND_EVENTS];
  ColorButton *lsq, *dsq, *textcb[11];
  TextPreview *preview;
  int FontBeingEdited;
  int jsval[5], jstate;

  static const char *FontSample[NFONTS];

  void Apply();
  void ApplyCheckBox(GtkWidget *cb,int *curval,int *ch1, int *ch2);
  void ApplyEntry(GtkWidget *entry,char *curval,int sz, int *ch1, int *ch2);
  void ApplyColorButton(ColorButton *cb,int *curval,int *ch1, int *ch2);

  void formatJoystickDescription();

  friend void prefs_ok(GtkWidget *w,gpointer data);
  friend void prefs_apply(GtkWidget *w,gpointer data);

  friend void prefs_frevert(GtkWidget *w,gpointer data);
  friend void prefs_cfont(GtkWidget *w,gpointer data);
  friend void prefs_fok(GtkWidget *w,gpointer data);
  friend void prefs_fcancel(GtkWidget *w,gpointer data);

  friend void prefs_sndtest(GtkWidget *w,gpointer data);
  friend void prefs_sndedit(GtkWidget *w,gpointer data);

  friend void prefs_defcolor(GtkWidget *w,gpointer data);

  friend void prefs_joyctl(GtkWidget *w,gpointer data);
};

void prefs_ok(GtkWidget *w,gpointer data);
void prefs_apply(GtkWidget *w,gpointer data);

void prefs_frevert(GtkWidget *w,gpointer data);
void prefs_cfont(GtkWidget *w,gpointer data);
void prefs_fok(GtkWidget *w,gpointer data);
void prefs_fcancel(GtkWidget *w,gpointer data);

void prefs_sndtest(GtkWidget *w,gpointer data);
void prefs_sndedit(GtkWidget *w,gpointer data);

void prefs_defcolor(GtkWidget *w,gpointer data);
void prefs_joyctl(GtkWidget *w,gpointer data);

#endif
