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

const char test_input[] = "test_input/day17.txt";
const char real_input[] = "real_input/day17.txt";

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

typedef struct s_cell {
	struct {
		struct s_cell* cell;
		size_t cost;
	} neighbours[14];
	size_t dist;
} cell;

typedef struct {
	cell* cell;
	size_t dist;
} queueelem;

typedef struct {
	queueelem* data;
	size_t len;
	size_t start;
	size_t cap;
} queue;

void enqueue(queue* q, cell* c, size_t d) {
	if (q->len + 1 > q->cap) {
		q->cap = q->cap ? q->cap * 2 : 1;
		q->data = realloc(q->data, q->cap * sizeof(*q->data));
	}
	q->data[q->len++] = (queueelem){.cell = c, .dist = d};
}

bool has_data(queue q) {
	return q.start < q.len;
}

queueelem dequeue(queue* q) {
	return q->data[q->start++];
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	const uint8_t* scanner = data;
	size_t w = 0, h = 0;
	for (; *scanner != '\n'; scanner++, w++);
	for (; *scanner; scanner++, h++)
		for (; *scanner && *scanner != '\n'; scanner++);

	// i = x + y * w + side * w * h
	// where side = 1 for horizontal and 0 for vertical
	cell* grid = malloc(w * h * 2 * sizeof(cell));
	memset(grid, 0, w * h * 2 * sizeof(cell));

	for (ssize_t x = 0; x < w; x++) {
		for (ssize_t y = 0; y < h; y++) {
			size_t cost = 0;
			cell* vert = &grid[x + y * w];
			cell* hori = &grid[x + y * w + w * h];
			vert->dist = SIZE_MAX;
			hori->dist = SIZE_MAX;
			// let's calculate the cost going down
			for (ssize_t off = 0, oy = y + 1; oy < h && off < 3; oy++, off++) {
				cost += data[x + oy * (w + 1)] - '0';
				// target needs to be a horizontal cell
				cell* tgt = &grid[x + oy * w + w * h];
				vert->neighbours[off].cell = tgt;
				vert->neighbours[off].cost = cost;
			}
			cost = 0;
			// let's calculate the cost going up
			for (ssize_t off = 0, oy = y - 1; oy >= 0 && off < 3; oy--, off++) {
				cost += data[x + oy * (w + 1)] - '0';
				// target needs to be a horizontal cell
				cell* tgt = &grid[x + oy * w + w * h];
				vert->neighbours[3 + off].cell = tgt;
				vert->neighbours[3 + off].cost = cost;
			}
			cost = 0;
			// let's calculate the cost going right
			for (ssize_t off = 0, ox = x + 1; ox < w && off < 3; ox++, off++) {
				cost += data[ox + y * (w + 1)] - '0';
				// target needs to be a vertical cell
				cell* tgt = &grid[ox + y * w];
				hori->neighbours[off].cell = tgt;
				hori->neighbours[off].cost = cost;
			}
			cost = 0;
			// let's calculate the cost going left
			for (ssize_t off = 0, ox = x - 1; ox >= 0 && off < 3; ox--, off++) {
				cost += data[ox + y * (w + 1)] - '0';
				// target needs to be a vertical cell
				cell* tgt = &grid[ox + y * w];
				hori->neighbours[3 + off].cell = tgt;
				hori->neighbours[3 + off].cost = cost;
			}
		}
	}

	cell start = { 0 };
	// start has right & down
	for (size_t i = 0; i < 3; i++) {
		// down
		start.neighbours[i] = grid[0].neighbours[i];
		// right
		start.neighbours[3 + i] = grid[w * h].neighbours[i];
	}
	start.dist = SIZE_MAX;

	queue q = { 0 };
	enqueue(&q, &start, 0);
	size_t iters = 0;
	while (has_data(q)) {
		queueelem curr = dequeue(&q);
		// current distance would be bigger or the same
		if (curr.cell->dist <= curr.dist) continue;
		curr.cell->dist = curr.dist;
		// iterate over neighbours
		for (size_t i = 0; i < 6; i++) {
			if (curr.cell->neighbours[i].cell) {
				size_t cost = curr.dist + curr.cell->neighbours[i].cost;
				if (curr.cell->neighbours[i].cell->dist <= cost) continue;
				enqueue(&q, curr.cell->neighbours[i].cell, cost);
			}
		}
	}

	size_t min = grid[w * h - 1].dist;
	if (grid[w * h * 2 - 1].dist < min) {
		min = grid[w * h * 2 - 1].dist;
	}
	printf("%zu\n", min);

	free(q.data);
	free(grid);
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	const uint8_t* scanner = data;
	size_t w = 0, h = 0;
	for (; *scanner != '\n'; scanner++, w++);
	for (; *scanner; scanner++, h++)
		for (; *scanner && *scanner != '\n'; scanner++);

	// i = x + y * w + side * w * h
	// where side = 1 for horizontal and 0 for vertical
	cell* grid = malloc(w * h * 2 * sizeof(cell));
	memset(grid, 0, w * h * 2 * sizeof(cell));

	for (ssize_t x = 0; x < w; x++) {
		for (ssize_t y = 0; y < h; y++) {
			size_t cost = 0;
			cell* vert = &grid[x + y * w];
			cell* hori = &grid[x + y * w + w * h];
			vert->dist = SIZE_MAX;
			hori->dist = SIZE_MAX;
			// let's calculate the cost going down
			for (ssize_t i = 0, off = 0, oy = y + 1; oy < h && off < 10; oy++, off++) {
				cost += data[x + oy * (w + 1)] - '0';
				// target needs to be a horizontal cell
				if (off >= 3) {
					cell* tgt = &grid[x + oy * w + w * h];
					vert->neighbours[i].cell = tgt;
					vert->neighbours[i].cost = cost;
					i++;
				}
			}
			cost = 0;
			// let's calculate the cost going up
			for (ssize_t i = 0, off = 0, oy = y - 1; oy >= 0 && off < 10; oy--, off++) {
				cost += data[x + oy * (w + 1)] - '0';
				// target needs to be a horizontal cell
				if (off >= 3) {
					cell* tgt = &grid[x + oy * w + w * h];
					vert->neighbours[7 + i].cell = tgt;
					vert->neighbours[7 + i].cost = cost;
					i++;
				}
			}
			cost = 0;
			// let's calculate the cost going right
			for (ssize_t i = 0, off = 0, ox = x + 1; ox < w && off < 10; ox++, off++) {
				cost += data[ox + y * (w + 1)] - '0';
				// target needs to be a vertical cell
				if (off >= 3) {
					cell* tgt = &grid[ox + y * w];
					hori->neighbours[i].cell = tgt;
					hori->neighbours[i].cost = cost;
					i++;
				}
			}
			cost = 0;
			// let's calculate the cost going left
			for (ssize_t i = 0, off = 0, ox = x - 1; ox >= 0 && off < 10; ox--, off++) {
				cost += data[ox + y * (w + 1)] - '0';
				// target needs to be a vertical cell
				if (off >= 3) {
					cell* tgt = &grid[ox + y * w];
					hori->neighbours[7 + i].cell = tgt;
					hori->neighbours[7 + i].cost = cost;
					i++;
				}
			}
		}
	}

	cell start = { 0 };
	// start has right & down
	for (size_t i = 0; i < 7; i++) {
		// down
		start.neighbours[i] = grid[0].neighbours[i];
		// right
		start.neighbours[7 + i] = grid[w * h].neighbours[i];
	}
	start.dist = SIZE_MAX;

	queue q = { 0 };
	enqueue(&q, &start, 0);
	size_t iters = 0;
	while (has_data(q)) {
		queueelem curr = dequeue(&q);
		// current distance would be bigger or the same
		if (curr.cell->dist <= curr.dist) continue;
		curr.cell->dist = curr.dist;
		// iterate over neighbours
		for (size_t i = 0; i < 14; i++) {
			if (curr.cell->neighbours[i].cell) {
				size_t cost = curr.dist + curr.cell->neighbours[i].cost;
				if (curr.cell->neighbours[i].cell->dist <= cost) continue;
				enqueue(&q, curr.cell->neighbours[i].cell, cost);
			}
		}
	}

	size_t min = grid[w * h - 1].dist;
	if (grid[w * h * 2 - 1].dist < min) {
		min = grid[w * h * 2 - 1].dist;
	}
	printf("%zu\n", min);

	free(q.data);
	free(grid);
}
