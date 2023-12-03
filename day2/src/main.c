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

const char test_input[] = "test_input/day2.txt";
const char real_input[] = "real_input/day2.txt";

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

const char* color_names[3] = {
	"red",
	"green",
	"blue",
};

void part_1(const char* path, const uint8_t* data) {
	// RGB order
	uint64_t max_colors[3] = {
		12, // red cubes
		13, // green cubes
		14, // blue cubes
	};
	printf("part 1: %s\n", path);
	uint64_t id_sum = 0;
	while (*data) {
		uint64_t colors[3] = {
			0,
			0,
			0,
		};
		// skip 'Game '
		data += 5;
		// read in ID
		uint64_t id = 0;
		while (isdigit(*data)) {
			id *= 10;
			id += *data - '0';
			data++;
		}
		while (*data != '\n' && *data) {
			data++; // skip ':' / ',' / ';'
			data++; // skip ' '
			uint64_t v = 0;
			while (isdigit(*data)) {
				v *= 10;
				v += *data - '0';
				data++;
			}
			data++;
			for (int i = 0; i < sizeof(color_names) / sizeof(color_names[0]); i++) {
				int len = strlen(color_names[i]);
				if (strncmp(color_names[i], (const char*)data, len) == 0) {
					data += len;
					if (v > colors[i])
						colors[i] = v;
					break;
				}
			}
		}
		if (*data == '\n') data++;

		bool ok = true;
		for (int i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
			if (colors[i] > max_colors[i]) {
				ok = false;
				break;
			}
		}
		if (ok) id_sum += id;
	}
	printf("%lu\n", id_sum);
}

void part_2(const char* path, const uint8_t* data) {
	// RGB order
	printf("part 2: %s\n", path);
	uint64_t pow_sum = 0;
	while (*data) {
		uint64_t colors[3] = {
			0,
			0,
			0,
		};
		// skip 'Game '
		data += 5;
		// read in ID
		uint64_t id = 0;
		while (isdigit(*data)) {
			id *= 10;
			id += *data - '0';
			data++;
		}
		while (*data != '\n' && *data) {
			data++; // skip ':' / ',' / ';'
			data++; // skip ' '
			uint64_t v = 0;
			while (isdigit(*data)) {
				v *= 10;
				v += *data - '0';
				data++;
			}
			data++;
			for (int i = 0; i < sizeof(color_names) / sizeof(color_names[0]); i++) {
				int len = strlen(color_names[i]);
				if (strncmp(color_names[i], (const char*)data, len) == 0) {
					data += len;
					if (v > colors[i])
						colors[i] = v;
					break;
				}
			}
		}
		if (*data == '\n') data++;
		pow_sum += (colors[0] * colors[1] * colors[2]);
	}
	printf("%lu\n", pow_sum);
}
