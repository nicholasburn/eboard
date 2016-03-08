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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include <gst/gst.h>

#include "sound.h"
#include "global.h"
#include "tstring.h"
#include "util.h"
#include "stl.h"

#include "config.h"
#include "eboard.h"

SoundEvent::SoundEvent() {
  type=INT_WAVE;
  Pitch=800;
  Duration=250;
  Count=1;
  memset(ExtraData, 0, 256);
  enabled = true;
}

SoundEvent SoundEvent::operator=(SoundEvent &se) {
  type=se.type;
  Pitch=se.Pitch;
  Duration=se.Duration;
  Count=se.Count;
  enabled = se.enabled;
  strcpy(ExtraData,se.ExtraData);
  return(*this);
}

int SoundEvent::operator==(SoundEvent &se) {
  if (enabled!=se.enabled) return 0;
  if (type!=se.type) return 0;
  if (Pitch!=se.Pitch) return 0;
  if (Duration!=se.Duration) return 0;
  if (Count!=se.Count) return 0;
  if (strcmp(ExtraData,se.ExtraData)) return 0;
  return 1;
}

int SoundEvent::operator!=(SoundEvent &se) {
  return(! (se==(*this)) );
}

void SoundEvent::read(tstring &rcline) {
  int t;
  static const char *sep=",\r\n";
  string *p;

  memset(ExtraData,0,256);

  t=rcline.tokenvalue(sep);
  switch(t) {
  case 0: // INT_WAVE
    type=INT_WAVE;
    Count=1;
    Pitch=rcline.tokenvalue(sep);
    Duration=rcline.tokenvalue(sep);
    p=rcline.token(sep); // Device value, deprecated
    Count=rcline.tokenvalue(sep);
    if (!Count) Count=1;
    enabled = rcline.tokenbool(sep,true);
    break;
  case 1: // EXT_WAVE
    type=EXT_WAVE;
    Pitch=Duration=0;
    p=rcline.token(sep); // Device value, deprecated
    p=rcline.token(sep); p->copy(ExtraData,255);
    enabled = rcline.tokenbool(sep,true);
    break;
  case 2: // EXT_PROGRAM
    type=EXT_PROGRAM;
    Pitch=Duration=0;
    p=rcline.token(sep); p->copy(ExtraData,255);
    enabled = rcline.tokenbool(sep,true);
    break;
  case 3: // PLAIN_BEEP
    type=PLAIN_BEEP;
    Pitch=Duration=0;
    rcline.token(sep); /* beep string */
    enabled = rcline.tokenbool(sep,true);
    break;
  default:
    cerr << _("[eboard] bad RC line\n");
  }
}

ostream & operator<<(ostream &s,  SoundEvent e) {
  switch(e.type) {
  case INT_WAVE:
    s << "0," << e.Pitch << ',' << e.Duration << ',';
    s << "default" << ',' << e.Count << ',' << (e.enabled?1:0);
    break;
  case EXT_WAVE:
    s << "1," << "default" << ',' << e.ExtraData;
    s << ',' << (e.enabled?1:0);
    break;
  case EXT_PROGRAM:
    s << "2," << e.ExtraData << ',' << (e.enabled?1:0);
    break;
  case PLAIN_BEEP:
    s << "3,beep" << ',' << (e.enabled?1:0);
    break;
  }
  return(s);
}

void SoundEvent::play() {

  if (type==INT_WAVE) {
    sine_beep();
    return;
  }

  if (type==PLAIN_BEEP) {
    printf("%c",7);
    fflush(stdout);
    return;
  }

  if (!fork()) {

    if (type==EXT_WAVE) {
      gstPlay();
      _exit(0);
    }

    if (type==EXT_PROGRAM) {
      close(1);
      close(2);
      execlp("/bin/sh","/bin/sh","-c",ExtraData,0);
      _exit(0);
    }

    _exit(0);
  }
}

void SoundEvent::gstPlay() {
  char gst_string[512];
  GstElement *pipeline;
  GstBus     *bus;
  GstMessage *msg;
  string input(ExtraData);

  if (input.empty()) return;
  char tmp[512], *ptr;
  ptr = realpath(input.c_str(), tmp);
  if (ptr==NULL) return;
  input = tmp;

  //printf("gstPlay=[%s]\n",input.c_str());

  memset(gst_string,0,512);
  snprintf(gst_string,511,"playbin2 uri=file://%s",input.c_str());

  pipeline = gst_parse_launch(gst_string, NULL);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(bus,GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  if (msg != NULL)
    gst_message_unref(msg);
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  
  //printf("gstPlay done\n");
}

// TODO
void SoundEvent::sine_beep() { }

#ifdef BOGUS
void SoundEvent::sine_beep() {
  unsigned int rate=44100; // Hz
  int interval;
  
  short int *wave;
  short int silence[128];
  int bl,i,ts,ec,sc;
  double r,s;

  snd_pcm_t *ph;
  snd_pcm_hw_params_t *hw;
  int hwa=0;

  interval=(120*rate)/1000; // 120 msec
  bl = (rate*Duration)/1000;

  ts = bl*Count + interval*(Count-1); // total samples
  ts += 128- ts%128;

  wave = (short int *)malloc(2 * ts);
  if (!wave) return;

  for(i=0;i<ts;i++) wave[i] = 0;
  for(i=0;i<128;i++) silence[i] = 0;

  sc = ((120*rate)/1000) / 128; // silence frames

  for(i=0;i<bl;i++) {
    r= i * ((double)Pitch / (double)rate);
    s= ((double)i / (double)bl);
    s= sin(M_PI*s)*0.80;
    wave[i]=(short int)(32000.0*s*sin(M_PI*2.0*r));
  }

  for(i=1;i<Count;i++)
    memcpy(&wave[i*(bl+interval)],wave,bl*2);

  // open device if needed (one time)
  ec = snd_pcm_open(&ph, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (ec < 0) {
    cerr << "[eboard] ALSA's snd_pcm_open failed: " << snd_strerror(ec) << endl;
    goto leave2;
  }

  // set format or goto leave1
  ec = snd_pcm_hw_params_malloc(&hw); 
  if (ec < 0) goto leave1;
  hwa = 1;
  ec = snd_pcm_hw_params_any(ph, hw);
  if (ec < 0) goto leave1;
  ec = snd_pcm_hw_params_set_access(ph, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (ec < 0) goto leave1;
  ec = snd_pcm_hw_params_set_format(ph, hw, SND_PCM_FORMAT_S16_LE);
  if (ec < 0) goto leave1;
  ec = snd_pcm_hw_params_set_rate_near(ph, hw, &rate, 0);
  if (ec < 0) goto leave1;
  ec = snd_pcm_set_params(ph, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, rate, 1, 500000);
  if (ec < 0) goto leave1;
  ec = snd_pcm_hw_params_set_channels(ph, hw, 1);
  if (ec < 0) goto leave1;
  ec = snd_pcm_hw_params (ph, hw);
  if (ec < 0) goto leave1;
  snd_pcm_hw_params_free(hw);
  hwa = 0;

  ec = snd_pcm_prepare(ph);
  if (ec < 0) goto leave1;

  snd_pcm_writei(ph,wave,ts);
  snd_pcm_drain(ph);

 leave1:
  if (ec < 0)
    cerr << "[eboard] ALSA error: " << snd_strerror(ec) << endl;
  if (hwa)
    snd_pcm_hw_params_free(hw);
  snd_pcm_close(ph);
 leave2:
  free(wave);
}
#endif // ALSA_DRIVER

char *SoundEvent::getDescription() {
  switch(type) {
  case INT_WAVE:
    snprintf(pvt,128,_("%d %s, %d Hz for %d msec"),
	     Count,(Count>1)?_("beeps"):
	                     _("beep"),Pitch,Duration);
    break;
  case EXT_WAVE:
    snprintf(pvt,128,_("play file %s"),ExtraData);
    break;
  case EXT_PROGRAM:
    snprintf(pvt,128,_("run %s"),ExtraData);
    break;
  case PLAIN_BEEP:
    snprintf(pvt,128,_("plain console beep"));
    break;
  default:
    g_strlcpy(pvt,_("nothing"),128);
  }
  return pvt;
}

void SoundEvent::edit(SoundEventChangeListener *listener) {
  (new SoundEventDialog(this,listener))->show();
}

// dialog

SoundEventDialog::SoundEventDialog(SoundEvent *src, SoundEventChangeListener *listener) : ModalDialog(N_("Sound Event")) {
  GtkWidget *v,*tf,*rh,*mh[4],*ml[4],*hs,*bb,*ok,*cancel,*test,*brw;
  GSList *rg;
  int i,j;
  GtkObject *pitch,*dur,*cou;
  GtkWidget *tl=0,*om=0,*cbh=0,*omm,*ommi;

  gtk_window_set_default_size(GTK_WINDOW(widget),480,340);

  obj=src;
  hearer=listener;

  v=gtk_vbox_new(FALSE,4);
  gtk_container_add(GTK_CONTAINER(widget),v);

  tf=gtk_frame_new(_("Event Type"));
  gtk_frame_set_shadow_type(GTK_FRAME(tf),GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start(GTK_BOX(v),tf,FALSE,FALSE,4);

  rh=gtk_vbox_new(FALSE,4);
  gtk_container_add(GTK_CONTAINER(tf),rh);

  rd[0]=gtk_radio_button_new_with_label( 0, _("Beep (need Pitch, Duration and Count)") );
  rg=gtk_radio_button_group(GTK_RADIO_BUTTON(rd[0]));
  rd[1]=gtk_radio_button_new_with_label(rg, _("Play Media File (need Filename)") );
  rg=gtk_radio_button_group(GTK_RADIO_BUTTON(rd[1]));
  rd[2]=gtk_radio_button_new_with_label(rg, _("Run Program (need Filename)") );
  rg=gtk_radio_button_group(GTK_RADIO_BUTTON(rd[2]));
  rd[3]=gtk_radio_button_new_with_label(rg, _("Console Beep") );

  for(i=0;i<4;i++) {
    gtk_box_pack_start(GTK_BOX(rh),rd[i],FALSE,FALSE,4);
    gshow(rd[i]);
  }

  mh[0]=gtk_hbox_new(FALSE,4);
  mh[1]=gtk_hbox_new(FALSE,4);
  mh[2]=gtk_hbox_new(FALSE,4);
  mh[3]=gtk_hbox_new(FALSE,4);

  ml[0]=gtk_label_new(_("Pitch (Hz):"));
  ml[1]=gtk_label_new(_("Duration (msec):"));
  ml[2]=gtk_label_new(_("File to play / Program to run:"));
  ml[3]=gtk_label_new(_("Count:"));

  brw=gtk_button_new_with_label(_(" Browse... "));

  pitch=gtk_adjustment_new((gfloat)(src->Pitch),50.0,2000.0,1.0,10.0,0.0);
  en[0]=gtk_spin_button_new(GTK_ADJUSTMENT(pitch),0.5,0);

  dur=gtk_adjustment_new((gfloat)(src->Duration),30.0,4000.0,10.0,100.0,0.0);
  en[1]=gtk_spin_button_new(GTK_ADJUSTMENT(dur),0.5,0);

  en[2]=gtk_entry_new(); // file/program

  cou=gtk_adjustment_new((gfloat)(src->Count),1.0,5.0,1.0,1.0,0);
  en[3]=gtk_spin_button_new(GTK_ADJUSTMENT(cou),0.5,0);

  j=global.SoundFiles.size();

  if (j) {
    cbh=gtk_hbox_new(FALSE,4);
    om=gtk_option_menu_new();
    omm=gtk_menu_new();

    for(i=0;i<j;i++) {
      ommi=gtk_menu_item_new_with_label(global.SoundFiles[i].c_str());
      gtk_signal_connect(GTK_OBJECT(ommi),"activate",
			 GTK_SIGNAL_FUNC(snddlg_picktheme),(gpointer)this);
      gtk_menu_shell_append(GTK_MENU_SHELL(omm),ommi);
      gshow(ommi);
      sthemes.push_back(ommi);
    }
    gtk_option_menu_set_menu(GTK_OPTION_MENU(om),omm);
    tl=gtk_label_new(_("Configured Sound Files:"));
  }

  gtk_box_pack_start(GTK_BOX(v),mh[0],TRUE,TRUE,4);

  // row: pitch - duration - count 
  gtk_box_pack_start(GTK_BOX(mh[0]),ml[0],FALSE,FALSE,4);
  gtk_box_pack_start(GTK_BOX(mh[0]),en[0],TRUE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(mh[0]),ml[1],FALSE,FALSE,4);
  gtk_box_pack_start(GTK_BOX(mh[0]),en[1],TRUE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(mh[0]),ml[3],FALSE,FALSE,4);
  gtk_box_pack_start(GTK_BOX(mh[0]),en[3],TRUE,TRUE,4);

  // row: file to play label
  gtk_box_pack_start(GTK_BOX(v),mh[2],FALSE,FALSE,4);
  gtk_box_pack_start(GTK_BOX(mh[2]),ml[2],FALSE,FALSE,4);

  // row: file entry + browse
  gtk_box_pack_start(GTK_BOX(v),mh[3],FALSE,FALSE,4);
  gtk_box_pack_start(GTK_BOX(mh[3]),en[2],TRUE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(mh[3]),brw,FALSE,FALSE,4);

  if (j) {
    gtk_box_pack_end(GTK_BOX(cbh),om,FALSE,FALSE,4);
    gtk_box_pack_end(GTK_BOX(cbh),tl,FALSE,FALSE,4);
    gtk_box_pack_start(GTK_BOX(v),cbh,FALSE,FALSE,4);
  }

  for(i=0;i<4;i++)
    Gtk::show(ml[i],en[i],mh[i],NULL);
  
  if (j)
    Gtk::show(cbh,tl,om,NULL);

  gshow(brw);

  hs=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(v),hs,FALSE,FALSE,4);

  bb=gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bb), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bb), 5);
  gtk_box_pack_start(GTK_BOX(v),bb,FALSE,FALSE,2);

  ok=gtk_button_new_with_label(_("Ok"));
  GTK_WIDGET_SET_FLAGS(ok,GTK_CAN_DEFAULT);
  test=gtk_button_new_with_label(_("Test"));
  GTK_WIDGET_SET_FLAGS(test,GTK_CAN_DEFAULT);
  cancel=gtk_button_new_with_label(_("Cancel"));
  GTK_WIDGET_SET_FLAGS(cancel,GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bb),ok,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(bb),test,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(bb),cancel,TRUE,TRUE,0);
  gtk_widget_grab_default(ok);

  Gtk::show(bb,ok,test,cancel,hs,rh,tf,v,NULL);
  setDismiss(GTK_OBJECT(cancel),"clicked");

  switch(src->type) {
  case INT_WAVE:
    gtset(GTK_TOGGLE_BUTTON(rd[0]), 1);
    break;
  case EXT_WAVE:
    gtset(GTK_TOGGLE_BUTTON(rd[1]), 1);
    break;
  case EXT_PROGRAM:
    gtset(GTK_TOGGLE_BUTTON(rd[2]), 1);
    break;
  case PLAIN_BEEP:
    gtset(GTK_TOGGLE_BUTTON(rd[3]), 1);
    break;
  }

  gtk_entry_set_text(GTK_ENTRY(en[2]),src->ExtraData);

  gtk_signal_connect(GTK_OBJECT(ok),"clicked",
		     GTK_SIGNAL_FUNC(snddlg_ok),(gpointer)(this));
  gtk_signal_connect(GTK_OBJECT(test),"clicked",
		     GTK_SIGNAL_FUNC(snddlg_test),(gpointer)(this));
  gtk_signal_connect(GTK_OBJECT(brw),"clicked",
		     GTK_SIGNAL_FUNC(snddlg_browse),(gpointer)(this));

}

void snddlg_picktheme(GtkMenuItem *w,gpointer data) {
  SoundEventDialog *me;
  EboardFileFinder eff;
  char z[512],zz[512];
  int i,j;

  me=(SoundEventDialog *)data;
  
  j=me->sthemes.size();

  for(i=0;i<j;i++)
    if (w == GTK_MENU_ITEM(me->sthemes[i])) {
      g_strlcpy(z,global.SoundFiles[i].c_str(),512);
      if (strlen(z)) {
	if (eff.find(z,zz))
	  strcpy(z,zz);
	gtk_entry_set_text(GTK_ENTRY(me->en[2]),z);
	gtset(GTK_TOGGLE_BUTTON(me->rd[1]),TRUE);
      }      
      return;
    }
}

void snddlg_browse(GtkWidget *w,gpointer data) {
  SoundEventDialog *me;
  FileDialog *fd;

  me=(SoundEventDialog *)data;
  fd=new FileDialog(_("Browse"));
  
  if (fd->run()) {
    gtk_entry_set_text(GTK_ENTRY(me->en[2]), fd->FileName);
  }
  delete fd;
}

void snddlg_ok(GtkWidget *w,gpointer data) {
  SoundEventDialog *me;
  me=(SoundEventDialog *)data;
  me->apply(me->obj);
  if (me->hearer)
    me->hearer->SoundEventChanged();
  me->release();
}

void snddlg_test(GtkWidget *w,gpointer data) {
  SoundEventDialog *me;
  SoundEvent foo;
  me=(SoundEventDialog *)data;
  me->apply(&foo);
  foo.play();
}

void SoundEventDialog::apply(SoundEvent *dest) {
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rd[0])))
    dest->type=INT_WAVE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rd[1])))
    dest->type=EXT_WAVE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rd[2])))
    dest->type=EXT_PROGRAM;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rd[3])))
    dest->type=PLAIN_BEEP;
  dest->Pitch=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(en[0]));
  dest->Duration=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(en[1]));
  dest->Count=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(en[3]));
  g_strlcpy(dest->ExtraData,gtk_entry_get_text(GTK_ENTRY(en[2])),256);
}

