#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <ctype.h>

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

const char test_input[] = "test_input/day3.txt";
const char real_input[] = "real_input/day3.txt";

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

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	// find the size of the schematic
	uint64_t width = 0;
	uint64_t height = 0;
	const uint8_t* scanner = data;
	while (*scanner != '\n') {
		width++;
		scanner++;
	}
	scanner = data;
	while (*scanner) {
		while (*scanner != '\n' && *scanner) {
			scanner++;
		}
		height++;
		if (*scanner == '\n') scanner++;
	}
	size_t s = sizeof(bool) * width * height;
	bool* grid = malloc(s);
	memset(grid, 0, s);
	// grid stores whether a cell is next to a symbol
	for (uint64_t y = 0; y < height; y++) {
		for (uint64_t x = 0; x < width; x++) {
			int sIdx = x + y * (width + 1);
			uint8_t c = data[sIdx];
			if (isdigit(c) || c == '.') {
				continue;
			}
			// SYMBOL! :D
			for (int xO = -1; xO <= 1; xO++) {
				for (int yO = -1; yO <= 1; yO++) {
					uint64_t xN = x + xO;
					uint64_t yN = y + yO;
					int dIdx = xN + yN * (width);
					if (xN > width || yN > height) continue;
					grid[dIdx] = true;
				}
			}
		}
	}

	uint64_t sum = 0;
	// let's scan for part numbers
	for (uint64_t y = 0; y < height; y++) {
		uint64_t part_num = 0;
		bool is_part_num = false;
		for (uint64_t x = 0; x < width; x++) {
			int sIdx = x + y * (width + 1);
			int gIdx = x + y * (width);
			uint8_t c = data[sIdx];
			if (isdigit(c)) {
				part_num *= 10;
				part_num += c - '0';
				if (grid[gIdx]) is_part_num = true;
			} else {
				if (is_part_num) {
					sum += part_num;
				}
				is_part_num = false;
				part_num = 0;
			}
		}
		if (is_part_num)
			sum += part_num;
	}

	free(grid);
	printf("%lu\n", sum);
}

typedef struct {
	uint64_t ratio;
	int neighbouring_numbers;
} gear;

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	// find the size of the schematic
	uint64_t width = 0;
	uint64_t height = 0;
	const uint8_t* scanner = data;
	while (*scanner != '\n') {
		width++;
		scanner++;
	}
	scanner = data;
	while (*scanner) {
		while (*scanner != '\n' && *scanner) {
			scanner++;
		}
		height++;
		if (*scanner == '\n') scanner++;
	}
	// let's count the number of gears & create a gear grid
	size_t s = sizeof(int) * width * height;
	int gear_count = 0;
	int* grid = malloc(s);
	memset(grid, 0, s);
	// grid stores whether a cell is next to a symbol
	for (uint64_t y = 0; y < height; y++) {
		for (uint64_t x = 0; x < width; x++) {
			int sIdx = x + y * (width + 1);
			uint8_t c = data[sIdx];
			if (c != '*') continue;
			// SYMBOL! :D
			for (int xO = -1; xO <= 1; xO++) {
				for (int yO = -1; yO <= 1; yO++) {
					uint64_t xN = x + xO;
					uint64_t yN = y + yO;
					int dIdx = xN + yN * (width);
					if (xN > width || yN > height) continue;
					grid[dIdx] = gear_count + 1; // 1-indexed to allow 0 as sentinel
				}
			}
			gear_count++;
		}
	}
	// let's now create a gear list
	size_t n = sizeof(gear) * gear_count;
	gear* gears = malloc(n);
	memset(gears, 0, n);
	for (int i = 0; i < n / sizeof(gear); i++) {
		// ratios should start at 1
		gears[i].ratio = 1;
	}

	// let's scan for part numbers
	for (uint64_t y = 0; y < height; y++) {
		uint64_t part_num = 0;
		int number_for = 0;
		for (uint64_t x = 0; x < width; x++) {
			int sIdx = x + y * (width + 1);
			int gIdx = x + y * (width);
			uint8_t c = data[sIdx];
			if (isdigit(c)) {
				part_num *= 10;
				part_num += c - '0';
				if (grid[gIdx]) number_for = grid[gIdx];
			} else {
				if (number_for) {
					gears[number_for - 1].ratio *= part_num;
					gears[number_for - 1].neighbouring_numbers++;
				}
				number_for = 0;
				part_num = 0;
			}
		}
		if (number_for) {
			gears[number_for - 1].ratio *= part_num;
			gears[number_for - 1].neighbouring_numbers++;
		}
	}

	uint64_t sum = 0;
	for (int i = 0; i < n / sizeof(gear); i++) {
		if (gears[i].neighbouring_numbers == 2) sum += gears[i].ratio;
	}

	free(grid);
	free(gears);
	printf("%lu\n", sum);
}
