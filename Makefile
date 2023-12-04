CDIRS = testing day1 day2 day3 day4

.PHONY : all release cproj $(CDIRS) clean

# generate debug build
all: cproj

cproj: $(CDIRS)

$(CDIRS):
	make -C $@ $(MAKECMDGOALS)
