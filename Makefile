all: spanread grabframe takePicture
spanread:
	$(MAKE) -C src/spanread $(MFLAGS)
grabframe:
	$(MAKE) -C src/grabframe $(MFLAGS)
takePicture:
	$(MAKE) -C src/takePicture $(MFLAGS)

clean:
	$(MAKE) clean -C src/spanread
	$(MAKE) clean -C src/grabframe
	$(MAKE) clean -C src/takePicture
