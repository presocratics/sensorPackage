all: span grabframe rmglitch proccam debayer rectify pybin syncTime span2slam mmerge
span:
	$(MAKE) -C src/pySpan $(MFLAGS)
grabframe:
	$(MAKE) -C src/grabframe $(MFLAGS)
rmglitch:
	$(MAKE) -C src/rmglitch $(MFLAGS)
proccam:
	$(MAKE) -C src/proccam $(MFLAGS)
debayer:
	$(MAKE) -C src/debayer $(MFLAGS)
rectify:
	$(MAKE) -C src/rectify $(MFLAGS)
pybin:
	$(MAKE) -C src/pybin $(MFLAGS)
syncTime:
	$(MAKE) -C src/syncTime $(MFLAGS)
span2slam:
	$(MAKE) -C src/live $(MFLAGS)
mmerge:
	$(MAKE) -C src/mmerge $(MFLAGS)

clean:
	$(MAKE) clean -C src/pySpan
	$(MAKE) clean -C src/grabframe
	$(MAKE) clean -C src/rmglitch
	$(MAKE) clean -C src/proccam
	$(MAKE) clean -C src/debayer
	$(MAKE) clean -C src/rectify
	$(MAKE) clean -C src/pybin
	$(MAKE) clean -C src/syncTime
	$(MAKE) clean -C src/live
	$(MAKE) clean -C src/mmerge
