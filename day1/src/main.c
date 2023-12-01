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


	int fd = open(fileName, O_RDONLY);

	struct stat sb;

	fstat(fd, &sb);

	data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "Could not memory map file: %s\n", fileName);
		exit(1);
	}

	return data;
}

const bool TRY_REAL_INPUT = true;

const char test_input[] = "test_input/day1.txt";
const char real_input[] = "real_input/day1.txt";

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
	uint64_t sum = 0;
	printf("part 1: %s\n", path);
	while (*data) {
		// 10 as sentinel value to ensure I only set `initial` once
		uint64_t initial = 10;
		uint64_t final = 0;

		// find the values until the line ends
		while (*data && *data != '\n') {
			uint8_t c = *data;
			data++;
			if (c >= '0' && c <= '9') {
				uint8_t v = c - '0';
				if (initial == 10) initial = v;
				final = v;
			}
		}
		// skip the newline
		if (*data) data++;

		sum += initial * 10 + final;
	}
	printf("    %lu\n", sum);
}

const char* words[] = {
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine"
};

uint8_t word_digit(const uint8_t* text_value, uint64_t* length) {
	uint8_t c = *text_value;
	*length = 0;
	if (c == '\n' || c == '\0') {
		if (c == '\n') *length = 1;
		return 0;
	}
	if (c >= '0' && c <= '9') {
		*length = 1;
		return c - '0';
	}
	for (uint8_t i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
		int len = strlen(words[i]);
		if (strncmp(words[i], (const char*)text_value, len) == 0) {
			*length = 1;
			return i + 1;
		}
	}
	*length = 1;
	return 255;
}

void part_2(const char* path, const uint8_t* data) {
	uint64_t sum = 0;
	printf("part 2: %s\n", path);
	while (*data) {
		// 10 as sentinel value to ensure I only set `initial` once
		uint64_t initial = 10;
		uint64_t final = 0;
		uint64_t length = 0;
		uint8_t d = 0;
		while ((d = word_digit(data, &length))) {
			data += length;
			if (d == 255) continue;
			if (initial == 10) initial = d;
			final = d;
		}
		data += length;

		sum += initial * 10 + final;
	}
	printf("    %lu\n", sum);
}
