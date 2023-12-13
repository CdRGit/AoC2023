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

const char test_input[] = "test_input/day13.txt";
const char real_input[] = "real_input/day13.txt";

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
	uint64_t sum = 0;
	while (*data) {
		const uint8_t* scanner = data;
		size_t w = 0, h = 0;
		while (*scanner != '\n') {
			w++;
			scanner++;
		}
		scanner++;
		h++;
		while (*scanner == '#' || *scanner == '.') {
			while (*scanner && *scanner != '\n') {
				scanner++;
			}
			if (*scanner) scanner++;
			h++;
		}
		bool* grid = malloc(w * h * sizeof(bool));
		// read in the data
		for (size_t y = 0, i = 0; y < h; y++) {
			for (size_t x = 0; x < w; x++, i++) {
				grid[i] = (*data++) == '#';
			}
			data++;
		}

		// find if it's vertically reflected?
		size_t v_col = 1;
		for (v_col = 1; v_col < h; v_col++) {
			bool mirrored = true;
			for (size_t x = 0; x < w; x++) {
				size_t ui = x + (v_col - 1) * w;
				size_t li = x + (v_col + 0) * w;
				if (grid[ui] != grid[li]) {
					mirrored = false;
					break;
				}
			}
			if (mirrored) {
				// candidate found
				// let's check if this is valid
				for (ssize_t y1 = v_col-1, y2 = v_col; y1 >= 0 && y2 < h; y1--, y2++) {
					for (size_t x = 0; x < w; x++) {
						size_t ui = x + y1 * w;
						size_t li = x + y2 * w;
						if (grid[ui] != grid[li]) {
							mirrored = false;
							goto check_complete_v;
						}
					}
				}
check_complete_v:
				if (mirrored) break;
			}
		}
		// find if it's horizontally reflected?
		size_t h_row = 1;
		for (h_row = 1; h_row < w; h_row++) {
			bool mirrored = true;
			for (size_t y = 0; y < h; y++) {
				size_t ui = y * w + (h_row - 1);
				size_t li = y * w + (h_row + 0);
				if (grid[ui] != grid[li]) {
					mirrored = false;
					break;
				}
			}
			if (mirrored) {
				// candidate found
				// let's check if this is valid
				for (ssize_t x1 = h_row-1, x2 = h_row; x1 >= 0 && x2 < w; x1--, x2++) {
					for (size_t y = 0; y < h; y++) {
						size_t ui = y * w + x1;
						size_t li = y * w + x2;
						if (grid[ui] != grid[li]) {
							mirrored = false;
							goto check_complete_h;
						}
					}
				}
check_complete_h:
				if (mirrored) break;
			}
		}

		if (h_row == w) {
			sum += v_col * 100;
		} else {
			sum += h_row;
		}

		free(grid);
		if (*data) data++;
	}
	printf("%lu\n", sum);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	uint64_t sum = 0;
	while (*data) {
		const uint8_t* scanner = data;
		size_t w = 0, h = 0;
		while (*scanner != '\n') {
			w++;
			scanner++;
		}
		scanner++;
		h++;
		while (*scanner == '#' || *scanner == '.') {
			while (*scanner && *scanner != '\n') {
				scanner++;
			}
			if (*scanner) scanner++;
			h++;
		}
		bool* grid = malloc(w * h * sizeof(bool));
		// read in the data
		for (size_t y = 0, i = 0; y < h; y++) {
			for (size_t x = 0; x < w; x++, i++) {
				grid[i] = (*data++) == '#';
			}
			data++;
		}

		size_t column = 0;
		for (size_t c = 1; c < w; c++) {
			int errors = 0;
			for (ssize_t x1 = c-1, x2 = c; x1 >= 0 && x2 < w; x1--, x2++) {
				for (size_t y = 0; y < h; y++) {
					size_t li = x1 + y * w;
					size_t ri = x2 + y * w;
					errors += grid[li] != grid[ri];
				}
			}
			if (errors == 1) column = c;
		}

		size_t row = 0;
		if (!column) {
			for (size_t r = 1; r < h; r++) {
				int errors = 0;
				for (ssize_t y1 = r-1, y2 = r; y1 >= 0 && y2 < h; y1--, y2++) {
					for (size_t x = 0; x < w; x++) {
						size_t li = x + y1 * w;
						size_t ri = x + y2 * w;
						errors += grid[li] != grid[ri];
					}
				}
				if (errors == 1) row = r;
			}
		}

		if (row) {
			sum += row * 100;
		} else {
			sum += column;
		}

		free(grid);
		if (*data) data++;
	}
	printf("%lu\n", sum);
}
