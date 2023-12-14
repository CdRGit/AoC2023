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

const char test_input[] = "test_input/day14.txt";
const char real_input[] = "real_input/day14.txt";

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

typedef enum {
	EMPTY,
	SQUARE,
	ROUND,
} cell;

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	const uint8_t* scanner = data;
	size_t w = 0, h = 0;
	while (*scanner != '\n') {
		w++;
		scanner++;
	}
	while (*scanner) {
		h++;
		while (*scanner && *scanner != '\n') {
			scanner++;
		}
		if (*scanner) scanner++;
	}

	cell* grid = malloc(w * h * sizeof(cell));

	// read in grid
	for (size_t i = 0, y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++, i++) {
			switch (*data++) {
				case '.':
					grid[i] = EMPTY;
					break;
				case '#':
					grid[i] = SQUARE;
					break;
				case 'O':
					grid[i] = ROUND;
					break;
			}
		}
		data++;
	}

	// slide north
	for (size_t y = 0, i = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++, i++) {
			// do we need to slide this guy?
			if (grid[i] == ROUND) {
				ssize_t target = y;
				grid[i] = EMPTY;
				for (target = y; target >= 0; target--) {
					if (grid[target * w + x] != EMPTY) break;
				}
				target++;
				grid[target * w + x] = ROUND;
			}
		}
	}

	uint64_t sum = 0;
	for (size_t i = 0, y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++, i++) {
			if (grid[i] == ROUND) {
				sum += (h - y);
			}
		}
	}

	free(grid);
	printf("%lu\n", sum);
}

typedef struct {
	uint64_t hash;
	size_t first_occurence;
} hashcell;

typedef struct {
	hashcell* data;
	size_t len;
	size_t cap;
} hashmap;

size_t find_or_add(hashmap* map, uint64_t hash, size_t first_occurence) {
	for (size_t i = 0; i < map->len; i++) {
		if (map->data[i].hash == hash) {
			return map->data[i].first_occurence;
		}
	}

	if (map->len + 1 > map->cap) {
		map->cap = map->cap ? map->cap * 2 : 1;
		map->data = realloc(map->data, sizeof(hashcell) * map->cap);
	}

	map->data[map->len++] = (hashcell){
		.hash = hash,
		.first_occurence = first_occurence,
	};

	return first_occurence;
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	const uint8_t* scanner = data;
	size_t w = 0, h = 0;
	while (*scanner != '\n') {
		w++;
		scanner++;
	}
	while (*scanner) {
		h++;
		while (*scanner && *scanner != '\n') {
			scanner++;
		}
		if (*scanner) scanner++;
	}

	cell* grid = malloc(w * h * sizeof(cell));

	// read in grid
	for (size_t i = 0, y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++, i++) {
			switch (*data++) {
				case '.':
					grid[i] = EMPTY;
					break;
				case '#':
					grid[i] = SQUARE;
					break;
				case 'O':
					grid[i] = ROUND;
					break;
			}
		}
		data++;
	}

	size_t goal = 1000000000;
	bool found_recurrence = false;
	hashmap recurrences = { 0 };

	// it's cycling time
	for (size_t c = 0; c < goal; c++) {
		// slide north
		for (size_t y = 0, i = 0; y < h; y++) {
			for (size_t x = 0; x < w; x++, i++) {
				// do we need to slide this guy?
				if (grid[i] == ROUND) {
					ssize_t target = y;
					grid[i] = EMPTY;
					for (target = y; target >= 0; target--) {
						if (grid[target * w + x] != EMPTY) break;
					}
					target++;
					grid[target * w + x] = ROUND;
				}
			}
		}
		// slide west
		for (size_t y = 0, i = 0; y < h; y++) {
			for (size_t x = 0; x < w; x++, i++) {
				// do we need to slide this guy?
				if (grid[i] == ROUND) {
					ssize_t target = x;
					grid[i] = EMPTY;
					for (target = x; target >= 0; target--) {
						if (grid[target + y * w] != EMPTY) break;
					}
					target++;
					grid[target + y * w] = ROUND;
				}
			}
		}
		// slide south
		for (size_t y = h - 1; y != UINT64_MAX; y--) {
			for (size_t x = 0; x < w; x++) {
				size_t i = x + y * w;
				// do we need to slide this guy?
				if (grid[i] == ROUND) {
					ssize_t target = y;
					grid[i] = EMPTY;
					for (target = y; target < h; target++) {
						if (grid[target * w + x] != EMPTY) break;
					}
					target--;
					grid[target * w + x] = ROUND;
				}
			}
		}
		// slide east
		for (size_t y = 0; y < h; y++) {
			for (size_t x = w - 1; x != UINT64_MAX; x--) {
				size_t i = x + y * w;
				// do we need to slide this guy?
				if (grid[i] == ROUND) {
					ssize_t target = x;
					grid[i] = EMPTY;
					for (target = x; target < w; target++) {
						if (grid[target + y * w] != EMPTY) break;
					}
					target--;
					grid[target + y * w] = ROUND;
				}
			}
		}
		// it's hashing time
		// this hash function probably sucks ass but we ball
		// it works on my input™
		uint64_t hash = 0;
		for (size_t i = 0; i < w * h; i++) {
			hash ^= (hash >> 63);
			hash ^= (grid[i] == ROUND) ? 0xED0C30DF73B32A34lu : 0x5061F296EE79C064lu;
			hash <<= 1;
		}
		size_t first_occurence = find_or_add(&recurrences, hash, c);
		if (first_occurence != c && !found_recurrence) {
			found_recurrence = true;
			size_t loop_len = c - first_occurence;
			size_t offset = (goal - first_occurence) % loop_len;
			// now to adjust the value to be correct
			c = goal - offset;
		}
	}

	uint64_t sum = 0;
	for (size_t i = 0, y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++, i++) {
			if (grid[i] == ROUND) {
				sum += (h - y);
			}
		}
	}

	free(recurrences.data);
	free(grid);
	printf("%lu\n", sum);
}
