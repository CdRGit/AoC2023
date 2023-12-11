#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdint.h>
uint8_t* read_data(const char* const fileName) {
	uint8_t* data;

	printf("reading: %s\n", fileName);

	int fd = open(fileName, O_RDONLY);

	struct stat sb;

	fstat(fd, &sb);
	printf("Size: %lu\n", (uint64_t)sb.st_size);

	data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "Could not memory map file: %s\n", fileName);
		exit(1);
	}

	return data;
}

const bool TRY_REAL_INPUT = true;

const char test_input[] = "test_input/day11.txt";
const char real_input[] = "real_input/day11.txt";

void part_1(const char* path, const uint8_t* data);
void part_2(const char* path, const uint8_t* data);

int main(int argc, const char** argv) {
	uint8_t* test_data = read_data(test_input);

	part_1(test_input, test_data);
	part_2(test_input, test_data);

	if (TRY_REAL_INPUT) {
		printf("---\n");
		uint8_t* real_data = read_data(real_input);
		part_1(real_input, real_data);
		part_2(real_input, real_data);
	}
}

typedef struct {
	int64_t x;
	int64_t y;
} ivec2;

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	// read in size
	// input width & height
	// this will not be the same as final width and height but we ball
	size_t iw = 0, ih = 0;
	const uint8_t* scanner = data;
	while (*scanner != '\n') {
		scanner++;
		iw++;
	}
	while (*scanner) {
		while (*scanner && *scanner != '\n') {
			scanner++;
		}
		if (*scanner) scanner++;
		ih++;
	}
	scanner = data;
	bool* in_grid = malloc(iw * ih * sizeof(bool));
	for (int idx = 0; idx < iw * ih; idx++) {
		if (*scanner == '\n') scanner++;
		in_grid[idx] = *scanner == '#';
		scanner++;
	}
	bool* expand_this = malloc((iw + ih) * sizeof(bool));
	// x-expand [0,iw)
	for (int x = 0; x < iw; x++) {
		bool galaxies = false;
		for (int y = 0; y < ih; y++) {
			if (in_grid[x + y * iw]) {
				galaxies = true;
				break;
			}
		}
		expand_this[x] = !galaxies;
	}
	size_t galaxy_count = 0;
	// y-expand [iw,iw+ih)
	for (int y = 0; y < ih; y++) {
		bool galaxies = false;
		for (int x = 0; x < iw; x++) {
			if (in_grid[x + y * iw]) {
				galaxies = true;
				galaxy_count++;
			}
		}
		expand_this[iw + y] = !galaxies;
	}
	int galaxy_idx = 0;
	ivec2* galaxies = malloc(sizeof(ivec2) * galaxy_count);
	for (int ix = 0, x = 0; ix < iw; ix++, x++) {
		// expand this column
		if (expand_this[ix]) x++;
		for (int iy = 0, y = 0; iy < ih; iy++, y++) {
			// expand this row
			if (expand_this[iw + iy]) y++;
			if (in_grid[ix + iy * iw]) {
				galaxies[galaxy_idx++] = (ivec2){.x = x, .y = y};
			}
		}
	}

	uint64_t sum = 0;
	for (int i = 0; i < galaxy_count; i++) {
		ivec2 s = galaxies[i];
		for (int j = i + 1; j < galaxy_count; j++) {
			ivec2 e = galaxies[j];
			int64_t dx = s.x - e.x;
			int64_t dy = s.y - e.y;
			int64_t dp = labs(dx) + labs(dy);
			sum += dp;
		}
	}

	printf("%lu\n", sum);

	free(galaxies);
	free(in_grid);
	free(expand_this);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	// read in size
	// input width & height
	// this will not be the same as final width and height but we ball
	size_t iw = 0, ih = 0;
	const uint8_t* scanner = data;
	while (*scanner != '\n') {
		scanner++;
		iw++;
	}
	while (*scanner) {
		while (*scanner && *scanner != '\n') {
			scanner++;
		}
		if (*scanner) scanner++;
		ih++;
	}
	scanner = data;
	bool* in_grid = malloc(iw * ih * sizeof(bool));
	for (int idx = 0; idx < iw * ih; idx++) {
		if (*scanner == '\n') scanner++;
		in_grid[idx] = *scanner == '#';
		scanner++;
	}
	bool* expand_this = malloc((iw + ih) * sizeof(bool));
	// x-expand [0,iw)
	for (int x = 0; x < iw; x++) {
		bool galaxies = false;
		for (int y = 0; y < ih; y++) {
			if (in_grid[x + y * iw]) {
				galaxies = true;
				break;
			}
		}
		expand_this[x] = !galaxies;
	}
	size_t galaxy_count = 0;
	// y-expand [iw,iw+ih)
	for (int y = 0; y < ih; y++) {
		bool galaxies = false;
		for (int x = 0; x < iw; x++) {
			if (in_grid[x + y * iw]) {
				galaxies = true;
				galaxy_count++;
			}
		}
		expand_this[iw + y] = !galaxies;
	}
	int galaxy_idx = 0;
	ivec2* galaxies = malloc(sizeof(ivec2) * galaxy_count);
	for (int64_t ix = 0, x = 0; ix < iw; ix++, x++) {
		// expand this column
		if (expand_this[ix]) x+=999999;
		for (int64_t iy = 0, y = 0; iy < ih; iy++, y++) {
			// expand this row
			if (expand_this[iw + iy]) y+=999999;
			if (in_grid[ix + iy * iw]) {
				galaxies[galaxy_idx++] = (ivec2){.x = x, .y = y};
			}
		}
	}

	uint64_t sum = 0;
	for (int i = 0; i < galaxy_count; i++) {
		ivec2 s = galaxies[i];
		for (int j = i + 1; j < galaxy_count; j++) {
			ivec2 e = galaxies[j];
			int64_t dx = s.x - e.x;
			int64_t dy = s.y - e.y;
			int64_t dp = labs(dx) + labs(dy);
			sum += dp;
		}
	}

	printf("%lu\n", sum);

	free(galaxies);
	free(in_grid);
	free(expand_this);
}
