CDIRS = testing day1 day2 day3 day4 day5 day6 day7

.PHONY : all release cproj $(CDIRS) clean

# generate debug build
all: cproj

cproj: $(CDIRS)

$(CDIRS):
	make -C $@ $(MAKECMDGOALS)
