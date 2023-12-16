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

const char test_input[] = "test_input/day16.txt";
const char real_input[] = "real_input/day16.txt";

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
	enum {
		EMPTY,
		MIRROR_RIGHT,
		MIRROR_LEFT,
		SPLITTER_VERT,
		SPLITTER_HORI,
	} type;
	uint8_t visited;
} tile;

typedef struct {
	size_t x,y;
	enum {
		UP,
		DOWN,
		LEFT,
		RIGHT,
	} direction;
} laser;

typedef struct {
	laser* data;
	size_t len;
	size_t cap;
} laser_stack;

void push(laser_stack* s, laser l) {
	if (s->len + 1 > s->cap) {
		s->cap = s->cap ? s->cap * 2 : 1;
		s->data = realloc(s->data, s->cap * sizeof(laser));
	}
	s->data[s->len++] = l;
}

bool has_data(laser_stack s) {
	return s.len != 0;
}

laser pop(laser_stack* s) {
	return s->data[--s->len];
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	// let's read in the grid size
	const uint8_t* scanner = data;
	size_t w = 0, h = 0;
	for (; *scanner != '\n'; scanner++, w++);
	for (; *scanner; h++) {
		for (; *scanner && *scanner != '\n'; scanner++);
		if (*scanner) scanner++;
	}
	tile* grid = malloc(w * h * sizeof(tile));

	for (size_t y = 0, i = 0; y < h; y++, data++) {
		for (size_t x = 0; x < w; x++, data++, i++) {
			grid[i].visited = 0;
			switch (*data) {
				case '.':
					grid[i].type = EMPTY;
					break;
				case '/':
					grid[i].type = MIRROR_RIGHT;
					break;
				case '\\':
					grid[i].type = MIRROR_LEFT;
					break;
				case '|':
					grid[i].type = SPLITTER_VERT;
					break;
				case '-':
					grid[i].type = SPLITTER_HORI;
					break;
				default:
					printf("'%c'???\n", *data);
					assert(false && "unexpected character");
					return;
			}
		}
	}

	laser_stack lasers = {0};
	push(&lasers, (laser){.x = 0, .y = 0, .direction = RIGHT});
	while (has_data(lasers)) {
		laser current = pop(&lasers);
		// out of bounds out of mind
		if (current.x >= w || current.y >= h) continue;

		uint8_t visit_dir = (uint8_t[4]){1, 2, 4, 8}[current.direction];
		if (grid[current.x + current.y * w].visited & visit_dir) {
			continue;
		}
		grid[current.x + current.y * w].visited |= visit_dir;

		switch (grid[current.x + current.y * w].type) {
			case EMPTY:
				switch (current.direction) {
					case RIGHT:
						push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
						break;
					case LEFT:
						push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
						break;
					case UP:
						push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
						break;
					case DOWN:
						push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
						break;
				}
				break;
			case MIRROR_RIGHT:
				switch (current.direction) {
					case RIGHT:
						push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
						break;
					case LEFT:
						push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
						break;
					case UP:
						push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
						break;
					case DOWN:
						push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
						break;
				}
				break;
			case MIRROR_LEFT:
				switch (current.direction) {
					case RIGHT:
						push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
						break;
					case LEFT:
						push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
						break;
					case UP:
						push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
						break;
					case DOWN:
						push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
						break;
				}
				break;
			case SPLITTER_VERT:
				switch (current.direction) {
					case RIGHT:
					case LEFT:
						push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
						push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
						break;
					case UP:
						push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
						break;
					case DOWN:
						push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
						break;
				}
				break;
			case SPLITTER_HORI:
				switch (current.direction) {
					case RIGHT:
						push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
						break;
					case LEFT:
						push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
						break;
					case UP:
					case DOWN:
						push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
						push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
						break;
				}
				break;
			default:
				printf("%c\n", "./\\|-"[grid[current.x + current.y * w].type]);
				assert(false && "todo");
		}
	}
	size_t energized = 0;

	for (size_t i = 0; i < w * h; i++) {
		energized += grid[i].visited ? 1 : 0;
	}

	printf("%zu\n", energized);

	free(grid);
	free(lasers.data);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	// let's read in the grid size
	const uint8_t* scanner = data;
	size_t w = 0, h = 0;
	for (; *scanner != '\n'; scanner++, w++);
	for (; *scanner; h++) {
		for (; *scanner && *scanner != '\n'; scanner++);
		if (*scanner) scanner++;
	}
	tile* grid = malloc(w * h * sizeof(tile));

	for (size_t y = 0, i = 0; y < h; y++, data++) {
		for (size_t x = 0; x < w; x++, data++, i++) {
			grid[i].visited = 0;
			switch (*data) {
				case '.':
					grid[i].type = EMPTY;
					break;
				case '/':
					grid[i].type = MIRROR_RIGHT;
					break;
				case '\\':
					grid[i].type = MIRROR_LEFT;
					break;
				case '|':
					grid[i].type = SPLITTER_VERT;
					break;
				case '-':
					grid[i].type = SPLITTER_HORI;
					break;
				default:
					printf("'%c'???\n", *data);
					assert(false && "unexpected character");
					return;
			}
		}
	}

	size_t max = 0;
	laser_stack entrances = {0};
	for (size_t y = 0; y < h; y++) {
		// all the side entrances
		push(&entrances, (laser){.x = 0, .y = y, .direction = RIGHT});
		push(&entrances, (laser){.x = w - 1, .y = y, .direction = LEFT});
	}
	for (size_t x = 0; x < w; x++) {
		// all the top/bottom entrances
		push(&entrances, (laser){.x = x, .y = 0, .direction = DOWN});
		push(&entrances, (laser){.x = x, .y = h - 1, .direction = UP});
	}

	laser_stack lasers = {0};

	while (has_data(entrances)) {
		push(&lasers, pop(&entrances));
		while (has_data(lasers)) {
			laser current = pop(&lasers);
			// out of bounds out of mind
			if (current.x >= w || current.y >= h) continue;

			uint8_t visit_dir = (uint8_t[4]){1, 2, 4, 8}[current.direction];
			if (grid[current.x + current.y * w].visited & visit_dir) {
				continue;
			}
			grid[current.x + current.y * w].visited |= visit_dir;

			switch (grid[current.x + current.y * w].type) {
				case EMPTY:
					switch (current.direction) {
						case RIGHT:
							push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
							break;
						case LEFT:
							push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
							break;
						case UP:
							push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
							break;
						case DOWN:
							push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
							break;
					}
					break;
				case MIRROR_RIGHT:
					switch (current.direction) {
						case RIGHT:
							push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
							break;
						case LEFT:
							push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
							break;
						case UP:
							push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
							break;
						case DOWN:
							push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
							break;
					}
					break;
				case MIRROR_LEFT:
					switch (current.direction) {
						case RIGHT:
							push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
							break;
						case LEFT:
							push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
							break;
						case UP:
							push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
							break;
						case DOWN:
							push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
							break;
					}
					break;
				case SPLITTER_VERT:
					switch (current.direction) {
						case RIGHT:
						case LEFT:
							push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
							push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
							break;
						case UP:
							push(&lasers, (laser){.x = current.x, .y = current.y - 1, .direction = UP});
							break;
						case DOWN:
							push(&lasers, (laser){.x = current.x, .y = current.y + 1, .direction = DOWN});
							break;
					}
					break;
				case SPLITTER_HORI:
					switch (current.direction) {
						case RIGHT:
							push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
							break;
						case LEFT:
							push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
							break;
						case UP:
						case DOWN:
							push(&lasers, (laser){.x = current.x + 1, .y = current.y, .direction = RIGHT});
							push(&lasers, (laser){.x = current.x - 1, .y = current.y, .direction = LEFT});
							break;
					}
					break;
				default:
					printf("%c\n", "./\\|-"[grid[current.x + current.y * w].type]);
					assert(false && "todo");
			}
		}
		size_t energized = 0;

		for (size_t i = 0; i < w * h; i++) {
			energized += grid[i].visited ? 1 : 0;
			grid[i].visited = 0;
		}

		if (energized > max) max = energized;
	}

	printf("%zu\n", max);

	free(grid);
	free(lasers.data);
	free(entrances.data);
}
