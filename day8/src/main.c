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

const char test_input[] = "test_input/day8.txt";
const char real_input[] = "real_input/day8.txt";

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
	int left;
	int right;
	bool exists;
} location;

int calc_loc(const uint8_t* name) {
	return
		(name[0] - 'A') * (26 * 26) +
		(name[1] - 'A') * (26) +
		(name[2] - 'A') * 1;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	const uint8_t* instructions = data;

	const uint8_t* loc_data = data;
	for (int i = 0; i < 2; i++)
	{
		while (*loc_data != '\n')
			loc_data++;
		loc_data++;
	}

	// 26 * 26 * 26 locations
	location locations[26*26*26] = { 0 };
	while (*loc_data) {
		int idx = calc_loc(loc_data);
		loc_data += 7; // "??? = ("
		int left = calc_loc(loc_data);
		loc_data += 5; // "???, "
		int right = calc_loc(loc_data);
		loc_data += 4; // "???)"
		if (*loc_data == '\n') {
			loc_data++;
		}
		locations[idx].left = left;
		locations[idx].right = right;
	}

	int loc = 0;
	int instr_idx = 0;
	uint64_t steps = 0;
	// loop while we aren't at 'ZZZ'
	while (loc != 25 * 26 * 26 + 25 * 26 + 25) {
		if (instructions[instr_idx] == '\n') {
			instr_idx = 0;
		}
		switch (instructions[instr_idx++]) {
			case 'L':
				loc = locations[loc].left;
				break;
			case 'R':
				loc = locations[loc].right;
				break;
		}
		steps++;
	}
	printf("%lu\n", steps);
}

typedef struct {
	uint64_t initial;
	uint64_t length;
	int visit_cnt;
} cycle;

typedef struct {
	cycle* data;
	size_t len;
	size_t cap;
} cyc_vec;

void push_cyc(cyc_vec* vec, cycle c) {
	if (vec->len + 1 > vec->cap) {
		vec->cap = vec->cap ? vec->cap * 2 : 1;
		vec->data = realloc(vec->data, vec->cap * sizeof(cycle));
	}
	vec->data[vec->len++] = c;
}

typedef struct {
	uint64_t* data;
	size_t len;
	size_t cap;
} u64_vec;

void push_u64(u64_vec* vec, uint64_t u) {
	if (vec->len + 1 > vec->cap) {
		vec->cap = vec->cap ? vec->cap * 2 : 1;
		vec->data = realloc(vec->data, vec->cap * sizeof(uint64_t));
	}
	vec->data[vec->len++] = u;
}

uint64_t gcd(uint64_t a, uint64_t b) {
	// fuck it let's loop
	if (a < b) {
		uint64_t t = a;
		a = b;
		b = t;
		// swap em so `a` is the bigger
	}
	for (uint64_t d = b; d > 0; d--) {
		if (a % d == 0 && b % d == 0) return d;
	}
	return 1;
}

uint64_t lcm(uint64_t a, uint64_t b) {
	return a * b / gcd(a, b);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	const uint8_t* instructions = data;
	int instr_len = strchr((const char*)instructions, '\n') - (const char*)instructions;

	const uint8_t* loc_data = data;
	for (int i = 0; i < 2; i++)
	{
		while (*loc_data != '\n')
			loc_data++;
		loc_data++;
	}

	// 26 * 26 * 26 locations
	location locations[26*26*26] = { 0 };
	while (*loc_data) {
		int idx = calc_loc(loc_data);
		loc_data += 7; // "??? = ("
		int left = calc_loc(loc_data);
		loc_data += 5; // "???, "
		int right = calc_loc(loc_data);
		loc_data += 4; // "???)"
		if (*loc_data == '\n') {
			loc_data++;
		}
		locations[idx].left = left;
		locations[idx].right = right;
		locations[idx].exists = true;
	}

	int locs[26*26] = {0};
	int loc_cnt = 0;
	for (int i = 0; i < 26*26; i++) {
		if (locations[i * 26].exists) {
			locs[loc_cnt++] = i * 26;
		}
	}
	cycle end_loops[26*26] = { 0 };
	cycle* cycles = malloc(26*26*26*instr_len * sizeof(cycle));
	// start from each location
	for (int i = 0; i < loc_cnt; i++) {
		int instr_idx = 0;
		// find the cycle
		memset(cycles, 0, 26*26*26*instr_len * sizeof(cycle));
		int loc = locs[i];
		// loop 'forever' with an increasing 'step'
		for (uint64_t j = 0; true; j++) {
			if (instr_idx >= instr_len) instr_idx = 0;
			// mark current location
			int cyc_idx = loc * instr_len + instr_idx;
			if (cycles[cyc_idx].visit_cnt >= 2) {
				break;
			}
			if (!cycles[cyc_idx].visit_cnt) {
				cycles[cyc_idx].initial = j;
			} else {
				cycles[cyc_idx].length = j - cycles[cyc_idx].initial;
				if (loc % 26 == 25) {
					if (cycles[cyc_idx].initial - (j - cycles[cyc_idx].initial) == 0) {
						end_loops[i] = cycles[cyc_idx];
					}
				}
			}
			cycles[cyc_idx].visit_cnt++;
			switch (instructions[instr_idx++]) {
				case 'L':
					loc = locations[loc].left;
					break;
				case 'R':
					loc = locations[loc].right;
					break;
			}
		}
	}
	free(cycles);
	uint64_t steps = end_loops[0].length;
	// LCM time, I do not like how this functions on the example data they gave, ughhhhhhhhh
	for (int i = 1; i < loc_cnt; i++) {
		steps = lcm(steps, end_loops[i].length);
	}
	// why must there be hidden constraints in the input data I hate this
	printf("%lu\n", steps);
}
