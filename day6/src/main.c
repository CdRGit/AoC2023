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

const char test_input[] = "test_input/day6.txt";
const char real_input[] = "real_input/day6.txt";

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
	uint64_t time;
	uint64_t distance;
} race_info;

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	race_info* info = NULL;
	size_t cnt = 0;
	size_t cap = 0;
	// skip "Time:"
	data += 5;
	// skip spaces
	while (*data == ' ') data++;
	while (*data != '\n') {
		uint64_t t = 0;
		while (isdigit(*data)) {
			t *= 10;
			t += (*data++) - '0';
		}
		// skip spaces
		while (*data == ' ') data++;
		if (cnt + 1 > cap) {
			cap = cap ? cap * 2 : 1;
			info = realloc(info, cap * sizeof(race_info));
		}
		info[cnt++].time = t;
	}

	// skip "\nDistance:"
	data += 10;
	// skip spaces
	while (*data == ' ') data++;
	for (int i = 0; i < cnt; i++) {
		uint64_t d = 0;
		while (isdigit(*data)) {
			d *= 10;
			d += (*data++) - '0';
		}
		// skip spaces
		while (*data == ' ') data++;
		info[i].distance = d;
	}

	uint64_t margin = 1;
	for (int i = 0; i < cnt; i++) {
		// part 1 calculations
		race_info race = info[i];
		int race_wins = 0;
		for (int t = 0; t <= race.time; t++) {
			int remaining_length = race.time - t;
			int total_distance = t * remaining_length;
			if (total_distance > race.distance)
				race_wins++;
		}
		margin *= race_wins;
	}
	printf("%lu\n", margin);
	free(info);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	race_info race = { 0 };
	// skip "Time:"
	data += 5;
	// skip spaces
	while (*data == ' ') data++;
	uint64_t t = 0;
	while (isdigit(*data)) {
		t *= 10;
		t += (*data++) - '0';
		while (*data == ' ') data++;
	}
	race.time = t;
	assert(*data == '\n' && "Race Time read in incorrectly");

	// skip "\nDistance:"
	data += 10;
	// skip spaces
	while (*data == ' ') data++;
	uint64_t d = 0;
	while (isdigit(*data)) {
		d *= 10;
		d += (*data++) - '0';
		while (*data == ' ') data++;
		while (*data == '\n') data++;
	}
	race.distance = d;
	assert(*data == '\0' && "Race Distance read in incorrectly");

	uint64_t race_wins = 0;
	for (uint64_t t = 0; t <= race.time; t++) {
		uint64_t remaining_length = race.time - t;
		uint64_t total_distance = t * remaining_length;
		if (total_distance > race.distance)
			race_wins++;
	}
	printf("%lu\n", race_wins);
}
