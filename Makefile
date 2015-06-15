all: span grabframe rmglitch proccam
span:
	$(MAKE) -C src/pySpan $(MFLAGS)
grabframe:
	$(MAKE) -C src/grabframe $(MFLAGS)
rmglitch:
	$(MAKE) -C src/rmglitch $(MFLAGS)
proccam:
	$(MAKE) -C src/proccam $(MFLAGS)

clean:
	$(MAKE) clean -C src/pySpan
	$(MAKE) clean -C src/grabframe
	$(MAKE) clean -C src/rmglitch
	$(MAKE) clean -C src/proccam
