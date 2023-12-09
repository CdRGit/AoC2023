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

const char test_input[] = "test_input/day9.txt";
const char real_input[] = "real_input/day9.txt";

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
	int64_t* data;
	size_t len;
	size_t cap;
} i64_vec;

void i64_push(i64_vec* vec, int64_t v) {
	if (vec->len + 1 > vec->cap) {
		vec->cap = vec->cap ? vec->cap * 2 : 1;
		vec->data = realloc(vec->data, vec->cap * sizeof(int64_t));
	}
	vec->data[vec->len++] = v;
}

int64_t calc_next_value(i64_vec initial) {
	assert(initial.len != 0);
	for (int i = 0; i < initial.len; i++) {
		if (initial.data[i] != 0) continue;
		if (i == initial.len - 1) {
			return 0;
		}
	}
	i64_vec derivative = { 0 };
	for (int i = 1; i < initial.len; i++) {
		i64_push(&derivative, initial.data[i] - initial.data[i - 1]);
	}
	int64_t next = calc_next_value(derivative);
	free(derivative.data);
	return initial.data[initial.len - 1] + next;
	assert(false);
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	i64_vec vec = { 0 };
	int64_t sum = 0;
	while (*data) {
		while (*data && *data != '\n') {
			int64_t v = 0;
			bool neg = (*data == '-');
			if (neg) data++;
			while (isdigit(*data)) {
				v *= 10;
				v += (*data++) - '0';
			}
			v *= neg ? -1 : 1;
			i64_push(&vec, v);

			if (*data == ' ') data++;
		}
		// skip the `\n`
		data++;

		sum += calc_next_value(vec);

		vec.len = 0;
	}
	free(vec.data);
	printf("%ld\n", sum);
}

int64_t calc_prev_value(i64_vec initial) {
	assert(initial.len != 0);
	for (int i = 0; i < initial.len; i++) {
		if (initial.data[i] != 0) continue;
		if (i == initial.len - 1) {
			return 0;
		}
	}
	i64_vec derivative = { 0 };
	for (int i = 1; i < initial.len; i++) {
		i64_push(&derivative, initial.data[i] - initial.data[i - 1]);
	}
	int64_t next = calc_prev_value(derivative);
	free(derivative.data);
	return initial.data[0] - next;
	assert(false);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	i64_vec vec = { 0 };
	int64_t sum = 0;
	while (*data) {
		while (*data && *data != '\n') {
			int64_t v = 0;
			bool neg = (*data == '-');
			if (neg) data++;
			while (isdigit(*data)) {
				v *= 10;
				v += (*data++) - '0';
			}
			v *= neg ? -1 : 1;
			i64_push(&vec, v);

			if (*data == ' ') data++;
		}
		// skip the `\n`
		data++;

		sum += calc_prev_value(vec);

		vec.len = 0;
	}
	free(vec.data);
	printf("%ld\n", sum);
}
