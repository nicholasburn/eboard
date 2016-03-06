

DATAFILES = Adventurer.png Altinov.png Berlin.png Dominical72.png \
Ebony.png Harlequin.png Lcd.png NutBolt.png Primepov.png SadGreen.png \
Utretch.png Whitney.png \
Beat1.wav  Beat2.wav  Boing1.wav Challenge1.wav Cling1.wav GameOver1.wav \
Tick1.wav ShallWePlay.wav Victory1.wav Voosh.wav themeconf.extras2

INSTALL = /usr/bin/install
RMF     = /bin/rm -f 

install:
	if [ -d $(DATADIR) ]; then for p in $(DATAFILES); do $(INSTALL) -m 0644 $$p $(DATADIR)/$$p ; done ; fi

uninstall:
	for p in $(DATAFILES); do if [ -f $(DATADIR)/$$p ] ; then $(RMF) $(DATADIR)/$$p ; fi ; done

