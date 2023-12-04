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

const char test_input[] = "test_input/day4.txt";
const char real_input[] = "real_input/day4.txt";

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
	int number;
	int copies;
	int* winning_numbers;
	size_t winning_numbers_cnt;
	size_t winning_numbers_cap;
	int* actual_numbers;
	size_t actual_numbers_cnt;
	size_t actual_numbers_cap;
} card;

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	int sum = 0;
	while (*data) {
		card c = { 0 };
		data += 4; // "Card"
		while (*data == ' ') data++; // skip spaces
		while (isdigit(*data)) {
			c.number *= 10;
			c.number += (*data++) - '0';
		}
		data++; // ":"

		while (*data == ' ') data++; // skip spaces until we get to first number

		while (*data != '|') {
			int winning = 0;
			while (isdigit(*data)) {
				winning *= 10;
				winning += (*data++) - '0';
			}
			if (c.winning_numbers_cnt + 1 > c.winning_numbers_cap) {
				if (c.winning_numbers_cap) {
					c.winning_numbers_cap *= 2;
				} else {
					c.winning_numbers_cap = 1;
				}
				c.winning_numbers = realloc(c.winning_numbers, c.winning_numbers_cap * sizeof(int));
			}
			c.winning_numbers[c.winning_numbers_cnt++] = winning;
			while (*data == ' ') data++; // skip spaces
		}

		data++; // "|"
		while (*data == ' ') data++; // skip spaces

		// read in card numbers
		while (*data != '\n' && *data != '\0') {
			int actual = 0;
			while (isdigit(*data)) {
				actual *= 10;
				actual += (*data++) - '0';
			}
			if (c.actual_numbers_cnt + 1 > c.actual_numbers_cap) {
				if (c.actual_numbers_cap) {
					c.actual_numbers_cap *= 2;
				} else {
					c.actual_numbers_cap = 1;
				}
				c.actual_numbers = realloc(c.actual_numbers, c.actual_numbers_cap * sizeof(int));
			}
			c.actual_numbers[c.actual_numbers_cnt++] = actual;
			while (*data == ' ') data++; // skip spaces
		}

		if (*data) data++; // '\n'

		// score the card
		int score = 0;

		for (int i = 0; i < c.actual_numbers_cnt; i++) {
			int actual = c.actual_numbers[i];
			for (int j = 0; j < c.winning_numbers_cnt; j++) {
				int winning = c.winning_numbers[j];
				if (actual == winning) {
					if (score) score *= 2;
					else score = 1;
				}
			}
		}

		free(c.winning_numbers);
		free(c.actual_numbers);

		sum += score;
	}
	printf("%d\n", sum);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	card* cards = NULL;
	size_t cards_cnt = 0;
	size_t cards_cap = 0;

	while (*data) {
		card c = { 0 };
		c.copies = 1;
		data += 4; // "Card"
		while (*data == ' ') data++; // skip spaces
		while (isdigit(*data)) {
			c.number *= 10;
			c.number += (*data++) - '0';
		}
		data++; // ":"

		while (*data == ' ') data++; // skip spaces until we get to first number

		while (*data != '|') {
			int winning = 0;
			while (isdigit(*data)) {
				winning *= 10;
				winning += (*data++) - '0';
			}
			if (c.winning_numbers_cnt + 1 > c.winning_numbers_cap) {
				if (c.winning_numbers_cap) {
					c.winning_numbers_cap *= 2;
				} else {
					c.winning_numbers_cap = 1;
				}
				c.winning_numbers = realloc(c.winning_numbers, c.winning_numbers_cap * sizeof(int));
			}
			c.winning_numbers[c.winning_numbers_cnt++] = winning;
			while (*data == ' ') data++; // skip spaces
		}

		data++; // "|"
		while (*data == ' ') data++; // skip spaces

		// read in card numbers
		while (*data != '\n' && *data != '\0') {
			int actual = 0;
			while (isdigit(*data)) {
				actual *= 10;
				actual += (*data++) - '0';
			}
			if (c.actual_numbers_cnt + 1 > c.actual_numbers_cap) {
				if (c.actual_numbers_cap) {
					c.actual_numbers_cap *= 2;
				} else {
					c.actual_numbers_cap = 1;
				}
				c.actual_numbers = realloc(c.actual_numbers, c.actual_numbers_cap * sizeof(int));
			}
			c.actual_numbers[c.actual_numbers_cnt++] = actual;
			while (*data == ' ') data++; // skip spaces
		}

		if (*data) data++; // '\n'

		if (cards_cnt + 1 > cards_cap) {
			if (cards_cap) {
				cards_cap *= 2;
			} else {
				cards_cap = 1;
			}
			cards = realloc(cards, cards_cap * sizeof(card));
		}
		cards[cards_cnt++] = c;
	}

	for (int i = 0; i < cards_cnt; i++) {
		// score the card
		int matching = 0;
		for (int j = 0; j < cards[i].actual_numbers_cnt; j++) {
			int actual = cards[i].actual_numbers[j];
			for (int k = 0; k < cards[i].winning_numbers_cnt; k++) {
				int winning = cards[i].winning_numbers[k];
				if (winning == actual) matching++;
			}
		}
		for (int j = 1; j <= matching; j++) {
			cards[i + j].copies += cards[i].copies;
		}
	}

	int total = 0;
	for (int i = 0; i < cards_cnt; i++) {
		total += cards[i].copies;
		free(cards[i].winning_numbers);
		free(cards[i].actual_numbers);
	}
	free(cards);
	printf("%d\n", total);
}
