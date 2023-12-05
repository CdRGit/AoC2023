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

const char test_input[] = "test_input/day5.txt";
const char real_input[] = "real_input/day5.txt";

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
} vec;

void append(vec* v, int64_t val) {
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(int64_t));
	}
	v->data[v->len++] = val;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	data += 7; // "seeds: "
	// two lists
	vec primary = {0};
	vec secondary = {0};
	// read in a list of seeds
	while (*data != '\n') {
		int64_t v = 0;
		while (isdigit(*data)) {
			v *= 10;
			v += *data - '0';
			data++;
		}
		append(&primary, v);
		append(&secondary, v);
		if (*data == ' ') data++;
	}

	// skip the '\n'
	data++;

	// perform the mappings
	// '-1' as sentinel for "moved"
	while (*data) {
		// skip the blank line
		data++;
		// skip the "foo-to-bar map:" line
		while (data[0] != '\n') data++;
		data++;
		// while we're not on an empty line
		while (data[0] != '\n' && data[0] != '\0') {
			int64_t dst = 0;
			while (isdigit(*data)) {
				dst *= 10;
				dst += *data - '0';
				data++;
			}
			data++;
			int64_t src = 0;
			while (isdigit(*data)) {
				src *= 10;
				src += *data - '0';
				data++;
			}
			data++;
			int64_t cnt = 0;
			while (isdigit(*data)) {
				cnt *= 10;
				cnt += *data - '0';
				data++;
			}
			if (*data)
				data++;
			for (int64_t i = 0; i < primary.len; i++) {
				int64_t l = primary.data[i];
				if (l >= src && l < src + cnt) {
					secondary.data[i] = l - src + dst;
					primary.data[i] = -1;
				}
			}
		}

		// copy over the unchanged ones
		for (int64_t i = 0; i < primary.len; i++) {
			if (primary.data[i] != -1) {
				int64_t l = primary.data[i];
				secondary.data[i] = l;
				primary.data[i] = -1;
			}
		}

		vec tmp = primary;
		primary = secondary;
		secondary = tmp;
	}

	int64_t min = INT64_MAX;

	for (int64_t i = 0; i < primary.len; i++) {
		int64_t l = primary.data[i];
		if (l < min) min = l;
	}

	printf("%ld\n", min);

	free(primary.data);
	free(secondary.data);
}

typedef struct {
	int64_t start;
	int64_t length;
} range;

typedef struct {
	range* data;
	size_t len;
	size_t cap;
} r_vec;

void r_append(r_vec* v, range val) {
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(range));
	}
	v->data[v->len++] = val;
}

// fuck me we need some ranges
void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	data += 7; // "seeds: "
	// two lists
	r_vec primary = {0};
	// read in a list of seeds
	while (*data != '\n') {
		int64_t s = 0;
		while (isdigit(*data)) {
			s *= 10;
			s += *data - '0';
			data++;
		}
		data++;

		int64_t c = 0;
		while (isdigit(*data)) {
			c *= 10;
			c += *data - '0';
			data++;
		}

		range v;
		v.start = s;
		v.length = c;

		r_append(&primary, v);
		if (*data == ' ') data++;
	}

	// skip the '\n'
	data++;

	// perform the mappings
	// '-1' as sentinel for "moved"
	while (*data) {
		r_vec secondary = {0};
		// skip the blank line
		data++;
		// skip the "foo-to-bar map:" line
		while (data[0] != '\n') data++;
		data++;
		// while we're not on an empty line
		while (data[0] != '\n' && data[0] != '\0') {
			int64_t dst = 0;
			while (isdigit(*data)) {
				dst *= 10;
				dst += *data - '0';
				data++;
			}
			data++;
			int64_t src = 0;
			while (isdigit(*data)) {
				src *= 10;
				src += *data - '0';
				data++;
			}
			data++;
			int64_t cnt = 0;
			while (isdigit(*data)) {
				cnt *= 10;
				cnt += *data - '0';
				data++;
			}
			if (*data)
				data++;
			for (int64_t i = 0; i < primary.len; i++) {
				range r = primary.data[i];
				// skip non-ranges
				if (r.start == -1) continue;
				// range overlap
				// there are a few types:
				// |-----|
				//         |-------|
				// [0] 1 in src, 0 in dest: no overlap, second further
				//          |-------|
				// |------|
				// [1] 1 in src, 0 in dest: no overlap, second earlier
				//      |-----|
				// |-----------------|
				// [2] 0 in src, 1 in dest: fully contained, second bigger
				// |-------------|
				//    |-------|
				// [3] 2 in src, 1 in dest: fully contained, second smaller
				// |-------|
				//      |----------|
				// [4] 1 in src, 1 in dest: partial overlap, second further
				//       |-------|
				// |-------|
				// [5] 1 in src, 1 in dest: partial overlap, second earlier

				// https://stackoverflow.com/a/3269471
				int64_t x1 = r.start;
				int64_t x2 = r.start + r.length - 1;
				int64_t y1 = src;
				int64_t y2 = src + cnt - 1;
				// no overlap, NOPs
				// [0] & [1]
				if (x1 > y2 || y1 > x2) {
					continue;
				// [2]
				} else if (x1 >= y1 && x2 <= y2) {
					primary.data[i].start = -1;
					r.start -= src;
					r.start += dst;
					r_append(&secondary, r);
				// [3]
				} else if (x1 < y1 && x2 > y2) {
					primary.data[i].length = y1 - x1;
					range n;
					r.start = y1;
					r.length = y2 - y1 + 1;
					n.start = y2 + 1;
					n.length = x2 - y2;
					r.start -= src;
					r.start += dst;
					r_append(&secondary, r);
					r_append(&primary, n);
				// [4]
				} else if (x1 <= y1 && x2 <= y2) {
					r.start = y1;
					r.length = x2 - y1 + 1;
					primary.data[i].length = y1 - x1;
					r.start -= src;
					r.start += dst;
					r_append(&secondary, r);
				// [5]
				} else if (x1 >= y1 && x2 >= y2) {
					primary.data[i].start = y2 + 1;
					primary.data[i].length = x2 - y2;

					r.length = y2 - x1 + 1;
					r.start -= src;
					r.start += dst;
					r_append(&secondary, r);
				}
			}
		}

		// copy over the unchanged ones
		for (int64_t i = 0; i < primary.len; i++) {
			if (primary.data[i].start != -1) {
				range t = primary.data[i];
				r_append(&secondary, t);
			}
		}

		free(primary.data);
		primary = secondary;
	}

	int64_t min = INT64_MAX;
	for (int64_t i = 0; i < primary.len; i++) {
		if (primary.data[i].start < min) {
			min = primary.data[i].start;
		}
	}

	printf("%ld\n", min);

	free(primary.data);
}
