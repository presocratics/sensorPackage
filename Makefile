all: span grabframe takePicture
span:
	$(MAKE) -C src/pySpan $(MFLAGS)
grabframe:
	$(MAKE) -C src/grabframe $(MFLAGS)
takePicture:
	$(MAKE) -C src/takePicture $(MFLAGS)

clean:
	$(MAKE) clean -C src/pySpan
	$(MAKE) clean -C src/grabframe
	$(MAKE) clean -C src/takePicture
