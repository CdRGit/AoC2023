CDIRS = testing day1

.PHONY : all release cproj $(CDIRS) clean

# generate debug build
all: cproj

cproj: $(CDIRS)

$(CDIRS):
	make -C $@ $(MAKECMDGOALS)
