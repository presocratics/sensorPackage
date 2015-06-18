all: span grabframe rmglitch proccam debayer rectify
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

clean:
	$(MAKE) clean -C src/pySpan
	$(MAKE) clean -C src/grabframe
	$(MAKE) clean -C src/rmglitch
	$(MAKE) clean -C src/proccam
	$(MAKE) clean -C src/debayer
	$(MAKE) clean -C src/rectify
