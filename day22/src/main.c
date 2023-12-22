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

const char test_input[] = "test_input/day22.txt";
const char real_input[] = "real_input/day22.txt";

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

typedef size_t vec3[3];
typedef struct {
	vec3 c1, c2;
	bool valid;
} block;
typedef struct {
	block* data;
	size_t len;
	size_t cap;
} blocks;
typedef struct {
	int* data;
	size_t len;
	size_t cap;
} int_vec;

void append_block(blocks* v, block b) {
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(block));
	}
	v->data[v->len++] = b;
}

void append_int(int_vec* v, int i) {
	for (int j = 0; j < v->len; j++) {
		// no duplicates
		if (v->data[j] == i) return;
	}
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(int));
	}
	v->data[v->len++] = i;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	blocks b = {0};
	vec3 min = {SIZE_MAX, SIZE_MAX, SIZE_MAX};
	vec3 max = {0};
	while (*data) {
		vec3 c1 = {0};
		vec3 c2 = {0};
		for (int i = 0; i < 3; i++) {
			c1[i] = 0;
			while (isdigit(*data)) {
				c1[i] *= 10;
				c1[i] += *data++ - '0';
			}
			data++;
		}
		for (int i = 0; i < 3; i++) {
			c2[i] = 0;
			while (isdigit(*data)) {
				c2[i] *= 10;
				c2[i] += *data++ - '0';
			}
			assert(c1[i] <= c2[i]);
			if (*data)
				data++;
		}
		for (int i = 0; i < 3; i++) {
			if (c1[i] < min[i])
				min[i] = c1[i];
			if (c2[i] > max[i])
				max[i] = c2[i];
		}
		append_block(&b, (block){.c1 = {c1[0], c1[1], c1[2]}, .c2 = {c2[0], c2[1], c2[2]}, .valid = true});
	}
	size_t width = max[0] + 1;
	size_t depth = max[1] + 1;
	size_t height = max[2] + 1;
	int* grid = malloc(width * depth * height * sizeof(int));
	for (size_t i = 0; i < width * depth * height; i++) {
		grid[i] = -1;
	}

	int_vec* supported_by = malloc(b.len * sizeof(int_vec));
	memset(supported_by, 0, b.len * sizeof(int_vec));

	// let's go up through the grid and see how far down each brick can move
	for (size_t z = 0; z < height; z++) {
		int curr = -1;
		for(;;) {
			bool found = false;
			// find the lowest index brick on the current layer, that's not the previously chosen brick
			for (int i = curr + 1; i < b.len; i++) {
				if (b.data[i].c1[2] == z) {
					curr = i;
					found = true;
					break;
				}
			}
			if (!found) break;
			block* brick = &b.data[curr];
			bool supported = false;
			size_t new_z = z;
			// how far down can we move this brick?
			for (ssize_t nz = z - 1; nz >= 1 && !supported; nz--) {
				for (size_t x = brick->c1[0]; x <= brick->c2[0]; x++) {
					for (size_t y = brick->c1[1]; y <= brick->c2[1]; y++) {
						size_t i = x + y * width + nz * width * depth;
						int j = grid[i];
						if (j != -1) {
							append_int(&supported_by[curr], j);
							supported = true;
						}
					}
				}
				if (!supported)
					new_z = nz;
			}
			if (!supported) {
				append_int(&supported_by[curr], -1);
			}
			// move brick to new position
			size_t height = brick->c2[2] - brick->c1[2];
			brick->c1[2] = new_z;
			brick->c2[2] = new_z + height;
			// add brick into grid
			for (size_t x = brick->c1[0]; x <= brick->c2[0]; x++) {
				for (size_t y = brick->c1[1]; y <= brick->c2[1]; y++) {
					for (size_t gz = brick->c1[2]; gz <= brick->c2[2]; gz++) {
						size_t i = x + y * width + gz * width * depth;
						grid[i] = curr;
					}
				}
			}
		}
	}

	size_t count = 0;

	for (int removed = 0; removed < b.len; removed++) {
		bool safe = true;
		for (int i = 0; i < b.len; i++) {
			if (i == removed) continue;
			if (supported_by[i].len == 1 && supported_by[i].data[0] == removed) {
				safe = false;
				break;
			}
		}
		if (safe) count++;
	}

	printf("%zu\n", count);

	for (int i = 0; i < b.len; i++) {
		free(supported_by[i].data);
	}
	free(grid);
	free(b.data);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	blocks b = {0};
	vec3 min = {SIZE_MAX, SIZE_MAX, SIZE_MAX};
	vec3 max = {0};
	while (*data) {
		vec3 c1 = {0};
		vec3 c2 = {0};
		for (int i = 0; i < 3; i++) {
			c1[i] = 0;
			while (isdigit(*data)) {
				c1[i] *= 10;
				c1[i] += *data++ - '0';
			}
			data++;
		}
		for (int i = 0; i < 3; i++) {
			c2[i] = 0;
			while (isdigit(*data)) {
				c2[i] *= 10;
				c2[i] += *data++ - '0';
			}
			assert(c1[i] <= c2[i]);
			if (*data)
				data++;
		}
		for (int i = 0; i < 3; i++) {
			if (c1[i] < min[i])
				min[i] = c1[i];
			if (c2[i] > max[i])
				max[i] = c2[i];
		}
		append_block(&b, (block){.c1 = {c1[0], c1[1], c1[2]}, .c2 = {c2[0], c2[1], c2[2]}, .valid = true});
	}
	size_t width = max[0] + 1;
	size_t depth = max[1] + 1;
	size_t height = max[2] + 1;
	int* grid = malloc(width * depth * height * sizeof(int));
	for (size_t i = 0; i < width * depth * height; i++) {
		grid[i] = -1;
	}

	int_vec* supported_by = malloc(b.len * sizeof(int_vec));
	memset(supported_by, 0, b.len * sizeof(int_vec));

	// let's go up through the grid and see how far down each brick can move
	for (size_t z = 0; z < height; z++) {
		int curr = -1;
		for(;;) {
			bool found = false;
			// find the lowest index brick on the current layer, that's not the previously chosen brick
			for (int i = curr + 1; i < b.len; i++) {
				if (b.data[i].c1[2] == z) {
					curr = i;
					found = true;
					break;
				}
			}
			if (!found) break;
			block* brick = &b.data[curr];
			bool supported = false;
			size_t new_z = z;
			// how far down can we move this brick?
			for (ssize_t nz = z - 1; nz >= 1 && !supported; nz--) {
				for (size_t x = brick->c1[0]; x <= brick->c2[0]; x++) {
					for (size_t y = brick->c1[1]; y <= brick->c2[1]; y++) {
						size_t i = x + y * width + nz * width * depth;
						int j = grid[i];
						if (j != -1) {
							append_int(&supported_by[curr], j);
							supported = true;
						}
					}
				}
				if (!supported)
					new_z = nz;
			}
			if (!supported) {
				append_int(&supported_by[curr], -1);
			}
			// move brick to new position
			size_t height = brick->c2[2] - brick->c1[2];
			brick->c1[2] = new_z;
			brick->c2[2] = new_z + height;
			// add brick into grid
			for (size_t x = brick->c1[0]; x <= brick->c2[0]; x++) {
				for (size_t y = brick->c1[1]; y <= brick->c2[1]; y++) {
					for (size_t gz = brick->c1[2]; gz <= brick->c2[2]; gz++) {
						size_t i = x + y * width + gz * width * depth;
						grid[i] = curr;
					}
				}
			}
		}
	}

	size_t sum = 0;
	int_vec removed_blocks = {0};

	// yes this is a quintuply nested loop
	// no I do not care anymore
	for (int removed = 0; removed < b.len; removed++) {
		removed_blocks.len = 0;
		append_int(&removed_blocks, removed);
		size_t removed_cnt = 0;
		while (removed_cnt != removed_blocks.len) {
			removed_cnt = removed_blocks.len;
			for (int i = 0; i < b.len; i++) {
				int support_count = supported_by[i].len;
				for (int j = 0; j < removed_blocks.len; j++) {
					if (removed_blocks.data[j] == i) break;
					for (int k = 0; k < supported_by[i].len; k++) {
						if (supported_by[i].data[k] == removed_blocks.data[j])
							support_count--;
					}
				}
				if (support_count == 0)
					append_int(&removed_blocks, i);
			}
		}
		sum += removed_cnt - 1;
	}

	printf("%zu\n", sum);

	for (int i = 0; i < b.len; i++) {
		free(supported_by[i].data);
	}
	free(grid);
	free(b.data);
}
