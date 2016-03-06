
DATAFILES = Alpha.png Diagram.png Kingdom.png Slics.png WCN.png \
            Book.png Dyche1.png Leipzig.png Staunton.png Yahoo.png \
            Chap.png Dyche2.png Mayan.png Traveler.png \
            ChessBase.png Enquil.png OldClassicDark.png Usual.png \
            Cmf.png Jumbo.png Rastapov.png Vision.png \
            Draw1.wav Move1.wav Tell1.wav themeconf.extras1

INSTALL = /usr/bin/install
RMF     = /bin/rm -f 

install:
	if [ -d $(DATADIR) ]; then for p in $(DATAFILES); do $(INSTALL) -m 0644 $$p $(DATADIR)/$$p ; done ; fi

uninstall:
	for p in $(DATAFILES); do if [ -f $(DATADIR)/$$p ] ; then $(RMF) $(DATADIR)/$$p ; fi ; done

