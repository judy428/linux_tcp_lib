
ifndef SUPPORTCMDS
SUPPORTCMDS:= all objs clean cleanall rebuild
export SUPPORTCMDS
endif

ifndef BOTTOM_LIB_TCP
BOTTOM_LIB_TCP:= $(shell pwd)
$(warning $(BOTTOM_LIB_TCP))
export BOTTOM_LIB_TCP
endif

SUBDIRS1:= \
	./base	\
	./common	\
	
.PHONY: SUPPORTCMDS

$(SUPPORTCMDS):
	for subdir in $(SUBDIRS1); do \
		echo "Making " $$subdir $@; \
		(cd $$subdir && make $@ || exit "$$?"); \
	done;
