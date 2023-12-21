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

const char test_input[] = "test_input/day21.txt";
const char real_input[] = "real_input/day21.txt";

void part_1(const char* path, const uint8_t* data);
void part_2(const char* path, const uint8_t* data);

int main(int argc, const char** argv) {
	uint8_t* test_data = read_data(test_input);

	part_1(test_input, test_data);
	printf("part 2 does not work on test data today\nN/A\n");

	if (TRY_REAL_INPUT) {
		printf("---\n");
		uint8_t* real_data = read_data(real_input);
		part_1(real_input, real_data);
		part_2(real_input, real_data);
	}
}

typedef struct {
	size_t x;
	size_t y;
	size_t timestamp;
} timestamped_vec2;

typedef struct {
	enum {
		GARDEN_PLOT,
		ROCK,
	} type;
	int first_reached_at;
} cell;

typedef struct {
	timestamped_vec2* data;
	size_t len;
	size_t cap;
	size_t start;
} queue;

void enqueue(queue* q, timestamped_vec2 v) {
	if (q->len + 1 > q->cap) {
		q->cap = q->cap ? q->cap * 2 : 1;
		q->data = realloc(q->data, q->cap * sizeof(timestamped_vec2));
	}
	q->data[q->len++] = v;
}

bool has_data(queue q) {
	return q.start != q.len;
}

timestamped_vec2 dequeue(queue* q) {
	return q->data[q->start++];
}

#define LIMIT 64

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	size_t width = 0, height = 0;
	timestamped_vec2 start = {0};
	const uint8_t* scanner = data;
	for (; *scanner != '\n'; scanner++, width++);
	for (; *scanner; scanner++, height++) {
		for (; *scanner != '\n'; scanner++);
	}
	cell* grid = malloc(width * height * sizeof(cell));

	for (size_t y = 0, i = 0; y < height; y++, data++) {
		for (size_t x = 0; x < width; x++, data++, i++) {
			grid[i].first_reached_at = -1;
			switch (*data) {
				case '#':
					grid[i].type = ROCK;
					break;
				case 'S':
					start.x = x;
					start.y = y;
				case '.':
					grid[i].type = GARDEN_PLOT;
					break;
			}
		}
	}

	queue q = {0};
	enqueue(&q, start);
	while (has_data(q)) {
		timestamped_vec2 curr = dequeue(&q);
		if (curr.x >= width || curr.y >= height) continue;
		size_t i = curr.x + curr.y * width;
		if (curr.timestamp > LIMIT) continue;
		if (grid[i].type == ROCK) continue;
		if (grid[i].first_reached_at != -1) continue;
		grid[i].first_reached_at = curr.timestamp;
		// NESW
		enqueue(&q, (timestamped_vec2){.x = curr.x, .y = curr.y - 1, .timestamp = curr.timestamp + 1});
		enqueue(&q, (timestamped_vec2){.x = curr.x + 1, .y = curr.y, .timestamp = curr.timestamp + 1});
		enqueue(&q, (timestamped_vec2){.x = curr.x, .y = curr.y + 1, .timestamp = curr.timestamp + 1});
		enqueue(&q, (timestamped_vec2){.x = curr.x - 1, .y = curr.y, .timestamp = curr.timestamp + 1});
	}

	size_t count = 0;

	for (int i = 0; i < width * height; i++) {
		if (grid[i].first_reached_at == -1) continue;
		if (grid[i].first_reached_at % 2 == LIMIT % 2) count++;
	}

	printf("%zu\n", count);

	free(q.data);
	free(grid);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	size_t width = 0, height = 0;
	timestamped_vec2 start[9] = {0};
	const uint8_t* scanner = data;
	for (; *scanner != '\n'; scanner++, width++);
	for (; *scanner; scanner++, height++) {
		for (; *scanner != '\n'; scanner++);
	}
	cell* grid = malloc(width * height * 9 * sizeof(cell));

	for (size_t y = 0, i = 0; y < height; y++, data++) {
		for (size_t x = 0; x < width; x++, data++, i++) {
			grid[i].first_reached_at = -1;
			switch (*data) {
				case '#':
					grid[i].type = ROCK;
					break;
				case 'S':
					start[0].x = x;
					start[0].y = y;
				case '.':
					grid[i].type = GARDEN_PLOT;
					break;
			}
			for (size_t j = 0; j < 9; j++) {
				grid[i + j * width * height] = grid[i];
			}
		}
	}

	// board order:
	// center, S, N, E, W, SE, SW, NE, NW

	// enter from the top
	start[1].x = start[0].x;
	start[1].y = 0;
	start[1].timestamp = 1;
	// enter from the bottom
	start[2].x = start[0].x;
	start[2].y = height - 1;
	start[2].timestamp = 1;
	// enter from the left
	start[3].x = 0;
	start[3].y = start[0].y;
	start[3].timestamp = 1;
	// enter from the right
	start[4].x = width - 1;
	start[4].y = start[0].y;
	start[4].timestamp = 1;
	// enter top-left
	start[5].x = 0;
	start[5].y = 0;
	start[5].timestamp = 1;
	// enter top-right
	start[6].x = width - 1;
	start[6].y = 0;
	start[6].timestamp = 1;
	// enter bottom-left
	start[7].x = 0;
	start[7].y = height - 1;
	start[7].timestamp = 1;
	// enter bottom-right
	start[8].x = width - 1;
	start[8].y = height - 1;
	start[8].timestamp = 1;

	assert(width == height);
	assert(width == 131);
	size_t edge_delay = (width - 1) / 2;
	size_t corner_delay = (width);
	size_t edge_pass_delay = width;
	size_t corner_pass_delay = (width * 2) - 1;

	size_t step_count = 26501365;

	size_t grid_max[9] = {0};
	queue q = {0};
	for (int board = 0; board < 9; board++) {
		q.len = 0;
		q.start = 0;
		enqueue(&q, start[board]);
		size_t max = 0;
		while (has_data(q)) {
			timestamped_vec2 curr = dequeue(&q);
			if (curr.x >= width || curr.y >= height) continue;
			size_t i = curr.x + curr.y * width + board * width * height;
			if (grid[i].type == ROCK) continue;
			if (grid[i].first_reached_at != -1) continue;
			grid[i].first_reached_at = curr.timestamp;
			max = curr.timestamp;
			// NESW
			enqueue(&q, (timestamped_vec2){.x = curr.x, .y = curr.y - 1, .timestamp = curr.timestamp + 1});
			enqueue(&q, (timestamped_vec2){.x = curr.x + 1, .y = curr.y, .timestamp = curr.timestamp + 1});
			enqueue(&q, (timestamped_vec2){.x = curr.x, .y = curr.y + 1, .timestamp = curr.timestamp + 1});
			enqueue(&q, (timestamped_vec2){.x = curr.x - 1, .y = curr.y, .timestamp = curr.timestamp + 1});
		}
		grid_max[board] = max;
	}

	// 65 + 131n
	size_t n = (step_count - 65) / 131;
	assert((step_count - 65) % 131 == 0);
	// amount of cardinal direction pieces at timestamp 131
	size_t cardinal_131 = 1;
	// amount of cardinal direction pieces at some large even timestamp
	size_t cardinal_even = (n / 2);
	// amount of cardinal direction pieces at some large odd timestamp
	size_t cardinal_odd = ((n - 1) / 2);
	// amount of corner pieces at timestamp 65
	size_t corner_65 = n;
	// amount of corner pieces at timestamp 196
	size_t corner_196 = (n - 1);
	// amount of corner pieces at some large even timestamp
	size_t corner_even = 0;
	// amount of corner pieces at some large odd timestamp
	size_t corner_odd = 0;
	for (ssize_t n_sub = n - 1; n_sub >= 2; n_sub-=2) {
		corner_even += (n_sub - 2);
		corner_odd += (n_sub - 1);
	}

	size_t base_score = 0;
	size_t cardinal_131_score = 0;
	size_t cardinal_even_score = 0;
	size_t cardinal_odd_score = 0;
	size_t corner_65_score = 0;
	size_t corner_196_score = 0;
	size_t corner_even_score = 0;
	size_t corner_odd_score = 0;
	for (size_t i = 0; i < width * height; i++) {
		// calculate all cardinal pieces at 131, even and odd
		for (size_t board = 1; board <= 4; board++) {
			int t = grid[i + board * width * height].first_reached_at;
			if (t == -1) continue;
			if (t % 2 == 1 && t <= 131) cardinal_131_score++;
			if (t % 2 == 0) cardinal_even_score++;
			if (t % 2 == 1) cardinal_odd_score++;
		}
		// calculate all corner pieces at 65, 196, even and odd
		for (size_t board = 5; board <= 8; board++) {
			int t = grid[i + board * width * height].first_reached_at;
			if (t == -1) continue;
			if (t % 2 == 1 && t <= 65) corner_65_score++;
			if (t % 2 == 0 && t <= 196) corner_196_score++;
			if (t % 2 == 0) corner_even_score++;
			if (t % 2 == 1) corner_odd_score++;
		}
		// calculate base board
		int t = grid[i].first_reached_at;
		if (t == -1) continue;
		if (t % 2 == step_count % 2) base_score++;
	}

	size_t score = base_score               +
		cardinal_131  * cardinal_131_score  +
		cardinal_even * cardinal_even_score +
		cardinal_odd  * cardinal_odd_score  +
		corner_65     * corner_65_score     +
		corner_196    * corner_196_score    +
		corner_even   * corner_even_score   +
		corner_odd    * corner_odd_score;

	printf("%zu\n", score);

	free(q.data);
	free(grid);
}
