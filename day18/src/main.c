#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <limits.h>

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

const char test_input[] = "test_input/day18.txt";
const char real_input[] = "real_input/day18.txt";

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
} vec2;

typedef struct {
	vec2* data;
	size_t len;
	size_t cap;
} vec_vector;

void append(vec_vector* v, vec2 p) {
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(vec2));
	}
	v->data[v->len++] = p;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	const uint8_t* scanner = data;
	int64_t x = 0, y = 0;

	vec_vector v = { 0 };

	int64_t perimeter = 0;
	while (*scanner) {
		// add position
		append(&v, (vec2){.x = x, .y = y});
		uint8_t direction = *scanner++;
		scanner++; // ' '
		uint64_t dist = 0;
		for (; *scanner != ' '; scanner++) {
			dist *= 10;
			dist += *scanner - '0';
		}
		perimeter += dist;
		switch (direction) {
			case 'R':
				x += dist;
				break;
			case 'L':
				x -= dist;
				break;
			case 'D':
				y += dist;
				break;
			case 'U':
				y -= dist;
				break;
			default:
				printf("'%c %lu'\n", direction, dist);
				assert(false && "TODO");
		}
		scanner += 10; // ' (#xxxxxx)'
		if (*scanner) scanner++;
	}

	// shoelace time
	int64_t sum1 = 0, sum2 = 0;
	for (int i = 0; i < v.len - 1; i++) {
		sum1 += (v.data[i].x) * (v.data[i + 1].y);
		sum2 += (v.data[i].y) * (v.data[i + 1].x);
	}
	// add the first & last vertices combined
	sum1 += (v.data[v.len - 1].x) * (v.data[0].y);
	sum2 += (v.data[v.len - 1].y) * (v.data[0].x);

	int64_t area = (sum1 - sum2) / 2;
	if (area < 0) area = -area;
	area += perimeter / 2 + 1;

	printf("%ld\n", area);

	free(v.data);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	const uint8_t* scanner = data;
	int64_t x = 0, y = 0;

	vec_vector v = { 0 };

	int64_t perimeter = 0;
	while (*scanner) {
		// add position
		append(&v, (vec2){.x = x, .y = y});
		while (*scanner++ != '#');
		// after '#'
		uint64_t dist = 0;
		for (int i = 0; i < 5; i++, scanner++) {
			dist *= 16;
			if (*scanner <= '9')
				dist += *scanner - '0';
			else
				dist += *scanner - 'a' + 0xA;
		}
		perimeter += dist;
		switch (*scanner++) {
			case '0':
				x += dist;
				break;
			case '2':
				x -= dist;
				break;
			case '1':
				y += dist;
				break;
			case '3':
				y -= dist;
				break;
			default:
				assert(false && "TODO");
		}
		scanner ++; // ')'
		if (*scanner) scanner++;
	}

	// shoelace time
	int64_t sum1 = 0, sum2 = 0;
	for (int i = 0; i < v.len - 1; i++) {
		sum1 += (v.data[i].x) * (v.data[i + 1].y);
		sum2 += (v.data[i].y) * (v.data[i + 1].x);
	}
	// add the first & last vertices combined
	sum1 += (v.data[v.len - 1].x) * (v.data[0].y);
	sum2 += (v.data[v.len - 1].y) * (v.data[0].x);

	int64_t area = (sum1 - sum2) / 2;
	if (area < 0) area = -area;
	area += perimeter / 2 + 1;

	printf("%ld\n", area);

	free(v.data);
}
