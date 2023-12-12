// show cache statistics at the end
#define CACHE_ANALYSIS

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

const char test_input[] = "test_input/day12.txt";
const char real_input[] = "real_input/day12.txt";

void part_1(const char* path, const uint8_t* data);
void part_2(const char* path, const uint8_t* data);

#ifdef CACHE_ANALYSIS
size_t cache_hits = 0;
size_t cache_misses = 0;
#endif

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

#ifdef CACHE_ANALYSIS
		printf("\ncache info:\n%zu hits, %zu misses\n", cache_hits, cache_misses);
#endif
}

typedef enum {
	OPERATIONAL,
	DAMAGED,
	UNKNOWN,
} condition;

typedef struct {
	condition* springs;
	size_t spring_cnt;
	size_t spring_cap;

	size_t unknown_cnt;

	uint64_t* counts;
	size_t count_cnt;
	size_t count_cap;
} record;

void push_spring(record* r, condition c) {
	if (r->spring_cnt + 1 > r->spring_cap) {
		r->spring_cap = r->spring_cap ? r->spring_cap * 2 : 1;
		r->springs = realloc(r->springs, r->spring_cap * sizeof(condition));
	}
	r->springs[r->spring_cnt++] = c;
	if (c == UNKNOWN) r->unknown_cnt++;
}

void push_count(record* r, uint64_t v) {
	if (r->count_cnt + 1 > r->count_cap) {
		r->count_cap = r->count_cap ? r->count_cap * 2 : 1;
		r->counts = realloc(r->counts, r->count_cap * sizeof(uint64_t));
	}
	r->counts[r->count_cnt++] = v;
}

int64_t* cache = NULL;
size_t cache_size = 0;
void init_cache(record r) {
	size_t sz = r.spring_cnt * r.count_cnt;
	if (sz > cache_size || !cache) {
		if (cache) {
			free(cache);
			cache = NULL;
		}
		cache = malloc(sz * sizeof(int64_t));
		cache_size = sz;
	}
	for (size_t i = 0; i < sz; i++) {
		cache[i] = -1;
	}
}

int64_t arrange(record r, size_t spring_idx, size_t count_idx) {
	// did we reach the end of r.counts? if so there's up to 1 arrangement, if the rest of the springs are all either damaged or unknown
	if (r.count_cnt == count_idx) {
		for (size_t i = spring_idx; i < r.spring_cnt; i++) {
			if (r.springs[i] == OPERATIONAL) {
				return 0; // this is not a valid arrangement
			}
		}
		return 1; // this is a valid arrangement
	}
	// did we reach the end of r.springs? if so there is an arrangement if and only if we also hit the end of r.counts
	// which we previously checked, so no arrangement
	if (r.spring_cnt == spring_idx) {
		return 0; // this is not a valid arrangement
	}
	// calculate cache idx
	size_t c_idx = spring_idx + count_idx * r.spring_cnt;
	// hit up the cache
	if (cache[c_idx] != -1) {
#ifdef CACHE_ANALYSIS
		cache_hits++;
#endif
		return cache[c_idx];
	}
#ifdef CACHE_ANALYSIS
	cache_misses++;
#endif

	int64_t arrangements = 0;
	// is this spring damaged? if so we recurse again and return that value instead
	while (r.springs[spring_idx] == DAMAGED) {
		arrangements = arrange(r, spring_idx + 1, count_idx);
		goto cache_value;
	}
	// we are now at an actually interesting spring
	if (r.springs[spring_idx] == UNKNOWN) {
		// if it's unknown we will first run through the next part pretending it to be at a damaged spring
		arrangements = arrange(r, spring_idx+1, count_idx);
	}
	// now it's either an operational spring or an unknown spring (we already handled the case where the unknown is damaged tho, so we can pretend it's just operational)
	size_t operational_count = 1;
	// skip to the next spring
	spring_idx++;
	// keep scanning, now we can see if this would be a legal position
	// if this spring is a damaged spring and the operational count != the current count, we return with the current number of arrangements
	// (either 0 if this spring was operational, or whatever the damaged case came up with)
	// if it's damaged and operational count == the current count we add the potential arrangements after that to the value and return it
	// if it's an unknown spring we check if the operational count == the current count, if so we say that spring is damaged and add its potential arrangements after
	// if operational count > the current count we return the current number of arrangements
	while (spring_idx < r.spring_cnt) {
		if (r.springs[spring_idx] == DAMAGED) {
			if (operational_count == r.counts[count_idx]) {
				// the arrangements after the spring
				arrangements += arrange(r, spring_idx + 1, count_idx + 1);
			}
			// damaged spring = done
			goto cache_value;
		} else if (r.springs[spring_idx] == OPERATIONAL) {
			operational_count++;
		} else {
			// unknown spring
			if (operational_count == r.counts[count_idx]) {
				// spring is deemed 'damaged'
				// get arrangements after the spring and then return
				arrangements += arrange(r, spring_idx + 1, count_idx + 1);
				goto cache_value;
			}
			// 'operational', keep going
			operational_count++;
		}
		spring_idx++;
		if (operational_count > r.counts[count_idx])
			goto cache_value;
	}
	// hit the end with some counts still left
	if (operational_count == r.counts[count_idx]) {
		// it is the right count tho!
		// so let's see if this is valid
		arrangements += arrange(r, spring_idx, count_idx+1);
	}
	// return the final value
cache_value:
	cache[c_idx] = arrangements;
	return arrangements;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	int64_t arrangements = 0;
	while (*data) {
		record current_line = { 0 };

		while (*data != ' ') {
			switch (*data++) {
				case '#':
					push_spring(&current_line, OPERATIONAL);
					break;
				case '.':
					push_spring(&current_line, DAMAGED);
					break;
				case '?':
					push_spring(&current_line, UNKNOWN);
					break;
			}
		}

		data++; // skip ' '
		while (*data && *data != '\n') {
			uint64_t v = 0;
			while (isdigit(*data)) {
				v *= 10;
				v += (*data++) - '0';
			}
			if (*data == ',') data++;
			push_count(&current_line, v);
		}

		// it's dynamic programming time
		init_cache(current_line);
		arrangements += arrange(current_line, 0, 0);

		free(current_line.springs);
		free(current_line.counts);
		if (*data) data++;
	}
	printf("%ld\n", arrangements);
	if (cache) {
		free(cache);
		cache = NULL;
	}
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	int64_t arrangements = 0;
	while (*data) {
		record current_line = { 0 };

		while (*data != ' ') {
			switch (*data++) {
				case '#':
					push_spring(&current_line, OPERATIONAL);
					break;
				case '.':
					push_spring(&current_line, DAMAGED);
					break;
				case '?':
					push_spring(&current_line, UNKNOWN);
					break;
			}
		}

		data++; // skip ' '
		while (*data && *data != '\n') {
			uint64_t v = 0;
			while (isdigit(*data)) {
				v *= 10;
				v += (*data++) - '0';
			}
			if (*data == ',') data++;
			push_count(&current_line, v);
		}

		// duplicate many a time
		size_t spring_cnt = current_line.spring_cnt;
		size_t count_cnt  = current_line.count_cnt;
		// 4 duplication times
		for (int i = 0; i < 4; i++) {
			// push the separator
			push_spring(&current_line, UNKNOWN);
			for (int j = 0; j < spring_cnt; j++) {
				push_spring(&current_line, current_line.springs[j]);
			}
			for (int j = 0; j < count_cnt; j++) {
				push_count(&current_line, current_line.counts[j]);
			}
		}

		// it's dynamic programming time
		init_cache(current_line);
		arrangements += arrange(current_line, 0, 0);

		free(current_line.springs);
		free(current_line.counts);
		if (*data) data++;
	}
	printf("%ld\n", arrangements);
	if (cache) {
		free(cache);
		cache = NULL;
	}
}
