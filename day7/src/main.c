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

const char test_input[] = "test_input/day7.txt";
const char real_input[] = "real_input/day7.txt";

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

typedef enum {
	C_ACE,
	C_KING,
	C_QUEEN,
	C_JACK,
	C_TEN,
	C_NINE,
	C_EIGHT,
	C_SEVEN,
	C_SIX,
	C_FIVE,
	C_FOUR,
	C_THREE,
	C_TWO,
	C_JOKER,
} CARD;

typedef enum {
	H_FIVE_OF_A_KIND,
	H_FOUR_OF_A_KIND,
	H_FULL_HOUSE,
	H_THREE_OF_A_KIND,
	H_TWO_PAIR,
	H_ONE_PAIR,
	H_HIGH_CARD,
} HAND_TYPE;

typedef struct {
	CARD cards[5];
	uint64_t bid;
	HAND_TYPE type;
	bool active;
} hand;

const char* LETTER_TO_CARD_MAP = "AKQJT98765432j";

HAND_TYPE classify_p1(hand h) {
	int card_types[14] = { 0 };
	for (int i = 0; i < 5; i++) {
		card_types[h.cards[i]]++;
	}
	int pair_count = 0;
	bool has_triple = false;
	bool has_quad = false;
	bool has_quint = false;
	for (int i = 0; i < 13; i++) {
		if (card_types[i] == 2) pair_count++;
		if (card_types[i] == 3) has_triple = true;
		if (card_types[i] == 4) has_quad = true;
		if (card_types[i] == 5) has_quint = true;
	}
	if (has_quint) return H_FIVE_OF_A_KIND;
	if (has_quad) return H_FOUR_OF_A_KIND;
	if (has_triple && pair_count >= 1) return H_FULL_HOUSE;
	if (has_triple) return H_THREE_OF_A_KIND;
	if (pair_count >= 2) return H_TWO_PAIR;
	if (pair_count) return H_ONE_PAIR;
	return H_HIGH_CARD;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	hand* hands = NULL;
	size_t cnt = 0;
	size_t cap = 0;
	while (*data) {
		if (cnt + 1 > cap) {
			cap = cap ? cap * 2 : 1;
			hands = realloc(hands, sizeof(hand) * cap);
		}
		hands[cnt].active = true;
		for (int i = 0; i < 5; i++) {
			char c = *data;
			CARD card = strchr(LETTER_TO_CARD_MAP, c) - LETTER_TO_CARD_MAP;
			hands[cnt].cards[i] = card;
			data++;
		}
		// classify the hand type
		hands[cnt].type = classify_p1(hands[cnt]);
		data++;
		hands[cnt].bid = 0;
		while (isdigit(*data)) {
			hands[cnt].bid *= 10;
			hands[cnt].bid += *data - '0';
			data++;
		}

		cnt++;

		if (*data == '\n') data++;
	}

	uint64_t winning = 0;
	for (int i = 0; i < cnt; i++) {
		int worst_j = 0;
		hand worst = { 0 };
		for (int j = 0; j < cnt; j++) {
			if (!hands[j].active) continue;
			if (!worst.active) {
				worst = hands[j];
				worst_j = j;
				continue;
			}
			hand current = hands[j];
			// is this worse than the current worst hand?
			if (current.type > worst.type) {
				// type is worse, we ball
				worst = current;
				worst_j = j;
			} else if (current.type == worst.type) {
				// type is the same, let's check the cards
				for (int i = 0; i < 5; i++) {
					if (current.cards[i] > worst.cards[i]) {
						worst_j = j;
						worst = current;
						break;
					} else if (current.cards[i] == worst.cards[i]) {
						continue;
					} else {
						break;
					}
				}
			}
		}
		hands[worst_j].active = false;
		winning += worst.bid * (i + 1);
	}
	printf("%lu\n", winning);
}

HAND_TYPE classify_p2(hand h) {
	int card_types[14] = { 0 };
	for (int i = 0; i < 5; i++) {
		card_types[h.cards[i]]++;
	}
	int pair_count = 0;
	bool has_triple = false;
	bool has_quad = false;
	bool has_quint = false;
	for (int i = 0; i < 13; i++) {
		if (card_types[i] == 2) pair_count++;
		if (card_types[i] == 3) has_triple = true;
		if (card_types[i] == 4) has_quad = true;
		if (card_types[i] == 5) has_quint = true;
	}
	// 11111
	if (has_quint) return H_FIVE_OF_A_KIND;
	// J1111
	if (has_quad && card_types[C_JOKER]) return H_FIVE_OF_A_KIND;
	// JJ111
	if (has_triple && card_types[C_JOKER] == 2) return H_FIVE_OF_A_KIND;
	// JJJ11
	if (pair_count && card_types[C_JOKER] == 3) return H_FIVE_OF_A_KIND;
	// JJJJ1 && JJJJJ
	if (card_types[C_JOKER] >= 4) return H_FIVE_OF_A_KIND;

	// 11112
	if (has_quad) return H_FOUR_OF_A_KIND;
	// J1112
	if (has_triple && card_types[C_JOKER]) return H_FOUR_OF_A_KIND;
	// JJ112
	if (pair_count && card_types[C_JOKER] == 2) return H_FOUR_OF_A_KIND;
	// JJJ12
	if (card_types[C_JOKER] >= 3) return H_FOUR_OF_A_KIND;

	// 11122
	if (has_triple && pair_count >= 1) return H_FULL_HOUSE;
	// J1122
	if (pair_count >= 2 && card_types[C_JOKER]) return H_FULL_HOUSE;

	// 11123
	if (has_triple) return H_THREE_OF_A_KIND;
	// J1123
	if (pair_count && card_types[C_JOKER]) return H_THREE_OF_A_KIND;
	// JJ123
	if (card_types[C_JOKER] >= 2) return H_THREE_OF_A_KIND;

	// 11223
	if (pair_count >= 2) return H_TWO_PAIR;

	// 11234
	if (pair_count) return H_ONE_PAIR;
	// J1234
	if (card_types[C_JOKER]) return H_ONE_PAIR;

	// 12345
	return H_HIGH_CARD;
}

const char* TYPE_NAMES[7] = {
	"Five of a kind",
	"Four of a kind",
	"Full house",
	"Three of a kind",
	"Two pair",
	"One pair",
	"High card",
};

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	hand* hands = NULL;
	size_t cnt = 0;
	size_t cap = 0;
	while (*data) {
		if (cnt + 1 > cap) {
			cap = cap ? cap * 2 : 1;
			hands = realloc(hands, sizeof(hand) * cap);
		}
		hands[cnt].active = true;
		for (int i = 0; i < 5; i++) {
			char c = *data;
			if (c == 'J') c = 'j';
			CARD card = strchr(LETTER_TO_CARD_MAP, c) - LETTER_TO_CARD_MAP;
			hands[cnt].cards[i] = card;
			data++;
		}
		// classify the hand type
		hands[cnt].type = classify_p2(hands[cnt]);
		data++;
		hands[cnt].bid = 0;
		while (isdigit(*data)) {
			hands[cnt].bid *= 10;
			hands[cnt].bid += *data - '0';
			data++;
		}

		cnt++;

		if (*data == '\n') data++;
	}

	uint64_t winning = 0;
	for (int i = 0; i < cnt; i++) {
		int worst_j = 0;
		hand worst = { 0 };
		for (int j = 0; j < cnt; j++) {
			if (!hands[j].active) continue;
			if (!worst.active) {
				worst = hands[j];
				worst_j = j;
				continue;
			}
			hand current = hands[j];
			// is this worse than the current worst hand?
			if (current.type > worst.type) {
				// type is worse, we ball
				worst = current;
				worst_j = j;
			} else if (current.type == worst.type) {
				// type is the same, let's check the cards
				for (int i = 0; i < 5; i++) {
					if (current.cards[i] > worst.cards[i]) {
						worst_j = j;
						worst = current;
						break;
					} else if (current.cards[i] == worst.cards[i]) {
						continue;
					} else {
						break;
					}
				}
			}
		}
		hands[worst_j].active = false;
		winning += worst.bid * (i + 1);
	}
	printf("%lu\n", winning);
}
