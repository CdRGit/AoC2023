CDIRS = testing day1 day2 day3 day4 day5 day6 day7 day8 day9 day10 day11 day12 day13 day14

.PHONY : all release cproj $(CDIRS) clean

# generate debug build
all: cproj

cproj: $(CDIRS)

$(CDIRS):
	make -C $@ $(MAKECMDGOALS)
