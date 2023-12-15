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

const char test_input[] = "test_input/day15.txt";
const char real_input[] = "real_input/day15.txt";

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
	printf("part 1: %s\n", path);

	uint64_t sum = 0;
	while (*data) {
		uint16_t curr = 0;

		while (*data && *data != ',') {
			curr += *data++;
			curr *= 17;
			curr &= 0xFF;
			while (*data == '\n') data++;
		}

		sum += curr;
		if (*data == ',') data++;
	}
	printf("%lu\n", sum);
}

typedef struct {
	uint8_t label[7];
	uint8_t focal_length;
} lens;

typedef struct {
	lens* data;
	size_t len;
	size_t cap;
} box;

void remove_lens(box* box, uint8_t label[7]) {
	for (size_t i = 0; i < box->len; i++) {
		if (strcmp((const char*)box->data[i].label, (const char*)label) == 0) {
			// copy things back and break
			for (size_t j = i; j < box->len - 1; j++) {
				box->data[j] = box->data[j+1];
			}
			box->len--;
			return;
		}
	}
}

void set_lens(box* box, lens l) {
	for (size_t i = 0; i < box->len; i++) {
		if (strcmp((const char*)box->data[i].label, (const char*)l.label) == 0) {
			// replace the lens
			box->data[i] = l;
			return;
		}
	}
	if (box->len + 1 > box->cap) {
		box->cap = box->cap ? box->cap * 2 : 1;
		box->data = realloc(box->data, box->cap * sizeof(lens));
	}
	box->data[box->len++] = l;
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);

	box boxes[256] = { 0 };

	uint64_t sum = 0;
	while (*data) {
		uint16_t idx = 0;
		lens l = {0};
		int label_index = 0;

		while (*data != '=' && *data != '-') {
			l.label[label_index++] = *data;
			idx += *data++;
			idx *= 17;
			idx &= 0xFF;
			while (*data == '\n') data++;
		}

		l.label[label_index] = '\0';

		if (*data++ == '=') {
			l.focal_length = (*data++) - '0';
			set_lens(&boxes[idx], l);
		} else {
			// we have '-'
			// let's see if we can remove a node
			remove_lens(&boxes[idx], l.label);
		}
		if (*data) data++;
	}

	for (size_t i = 0; i < 256; i++) {
		// it's boxing time
		for (size_t j = 0; j < boxes[i].len; j++) {
			// it's lensing time
			sum += (i + 1) * (j + 1) * boxes[i].data[j].focal_length;
		}
		if (boxes[i].data) free(boxes[i].data);
	}

	printf("%lu\n", sum);
}
