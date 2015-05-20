all: span grabframe 
span:
	$(MAKE) -C src/pySpan $(MFLAGS)
grabframe:
	$(MAKE) -C src/grabframe $(MFLAGS)

clean:
	$(MAKE) clean -C src/pySpan
	$(MAKE) clean -C src/grabframe
