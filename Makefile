all: spanread grabframe
spanread:
	$(MAKE) -C src/spanread $(MFLAGS)
grabframe:
	$(MAKE) -C src/grabframe $(MFLAGS)

clean:
	$(MAKE) clean -C src/spanread
	$(MAKE) clean -C src/grabframe

