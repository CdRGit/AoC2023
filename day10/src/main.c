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

const char test_input[] = "test_input/day10.txt";
const char real_input[] = "real_input/day10.txt";

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
	P_GRND,
	P_VERT,
	P_HORI,
	P_NE90,
	P_NW90,
	P_SW90,
	P_SE90,
	P_STRT,
} pipe;

const uint8_t* const chr_to_pipe = (const uint8_t*)".|-LJ7FSIO";

typedef struct {
	pipe p;
	int64_t dist;
} tile;

typedef struct {
	size_t x;
	size_t y;
	size_t dist;
} ivec3;

typedef struct {
	ivec3* data;
	size_t end;
	size_t start;
	size_t cap;
} ivec3_queue;

void enqueue(ivec3_queue* queue, ivec3 v) {
	if (queue->end + 1 > queue->cap) {
		queue->cap = queue->cap ? queue->cap * 2 : 1;
		queue->data = realloc(queue->data, queue->cap * sizeof(ivec3));
	}
	queue->data[queue->end++] = v;
}

ivec3 dequeue(ivec3_queue* queue) {
	return queue->data[queue->start++];
}

bool has_data(ivec3_queue queue) {
	return queue.start < queue.end;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	// read in size of the block
	const uint8_t* scanner = data;
	size_t w = 0;
	size_t h = 0;
	while (*scanner != '\n') {
		w++;
		scanner++;
	}
	while (*scanner) {
		while (*scanner && *scanner != '\n') {
			scanner++;
		}
		if (*scanner) scanner++;
		h++;
	}
	tile* grid = malloc(w * h * sizeof(tile));
	scanner = data;
	size_t idx = 0;

	ivec3 start = { 0 };

	while (*scanner) {
		while (*scanner && *scanner != '\n') {
			grid[idx].p = strchr((const char*)chr_to_pipe, *scanner) - (const char*)chr_to_pipe;
			if (*scanner == 'S') {
				start.x = idx % w;
				start.y = idx / w;
			}
			grid[idx++].dist = -1;
			scanner++;
		}
		if (*scanner) scanner++;
	}

	// scan over the grid to calculate the distances
	// BFS?
	// we need a queue
	// I wish I wasn't using C rn, FML
	ivec3_queue queue = {0};
	// this queue will never shrink, but we ball
	// I've got enough memory
	enqueue(&queue, start);
	while (has_data(queue)) {
		ivec3 curr = dequeue(&queue);
		size_t idx = curr.x + curr.y * w;
		// if the distance != -1 we move on
		if (grid[idx].dist != -1) continue;
		grid[idx].dist = curr.dist;
		switch (grid[idx].p) {
			case P_STRT: {
				// find the neighbouring pipes
				// they have to face this way
				size_t north = curr.x + (curr.y - 1) * w;
				if (curr.y) {
					// we have a `north`
					switch (grid[north].p) {
						case P_VERT:
						case P_SE90:
						case P_SW90:
							enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				size_t south = curr.x + (curr.y + 1) * w;
				if (curr.y < h) {
					// we have a `south`
					switch (grid[south].p) {
						case P_VERT:
						case P_NE90:
						case P_NW90:
							enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				size_t west = (curr.x - 1) + curr.y * w;
				if (curr.x) {
					// we have a `west`
					switch (grid[west].p) {
						case P_HORI:
						case P_SE90:
						case P_NE90:
							enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				size_t east = (curr.x + 1) + curr.y * w;
				if (curr.x < w) {
					// we have a `east`
					switch (grid[east].p) {
						case P_HORI:
						case P_SW90:
						case P_NW90:
							enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
			} break;
			case P_HORI: {
				enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_VERT: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = curr.dist + 1});
			} break;
			case P_NE90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_NW90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_SE90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_SW90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			default:
				printf("%c\n", chr_to_pipe[grid[idx].p]);
				assert(false);
		}
	}

	int64_t max = 0;
	for (size_t y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++) {
			if (grid[x + y * w].dist > max) {
				max = grid[x + y * w].dist;
			}
		}
	}

	printf("%ld\n", max);
	free(grid);
	free(queue.data);
}

typedef enum {
	INSIDE,
	WALL,
	OUTSIDE,
} subtile;

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	// read in size of the block
	const uint8_t* scanner = data;
	size_t w = 0;
	size_t h = 0;
	while (*scanner != '\n') {
		w++;
		scanner++;
	}
	while (*scanner) {
		while (*scanner && *scanner != '\n') {
			scanner++;
		}
		if (*scanner) scanner++;
		h++;
	}
	tile* grid = malloc(w * h * sizeof(tile));
	scanner = data;
	size_t idx = 0;

	ivec3 start = { 0 };

	while (*scanner) {
		while (*scanner && *scanner != '\n') {
			grid[idx].p = strchr((const char*)chr_to_pipe, *scanner) - (const char*)chr_to_pipe;
			if (*scanner == 'S') {
				start.x = idx % w;
				start.y = idx / w;
			}
			grid[idx++].dist = -1;
			scanner++;
		}
		if (*scanner) scanner++;
	}
	ivec3_queue queue = {0};
	enqueue(&queue, start);
	while (has_data(queue)) {
		ivec3 curr = dequeue(&queue);
		size_t idx = curr.x + curr.y * w;
		// if the distance != -1 we move on
		if (grid[idx].dist != -1) continue;
		grid[idx].dist = curr.dist;
		switch (grid[idx].p) {
			case P_STRT: {
				int dirs = 0;
				// find the neighbouring pipes
				// they have to face this way
				size_t north = curr.x + (curr.y - 1) * w;
				if (curr.y) {
					// we have a `north`
					switch (grid[north].p) {
						case P_VERT:
						case P_SE90:
						case P_SW90:
							dirs |= 1;
							enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				size_t south = curr.x + (curr.y + 1) * w;
				if (curr.y < h) {
					// we have a `south`
					switch (grid[south].p) {
						case P_VERT:
						case P_NE90:
						case P_NW90:
							dirs |= 2;
							enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				size_t west = (curr.x - 1) + curr.y * w;
				if (curr.x) {
					// we have a `west`
					switch (grid[west].p) {
						case P_HORI:
						case P_SE90:
						case P_NE90:
							dirs |= 4;
							enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				size_t east = (curr.x + 1) + curr.y * w;
				if (curr.x < w) {
					// we have a `east`
					switch (grid[east].p) {
						case P_HORI:
						case P_SW90:
						case P_NW90:
							dirs |= 8;
							enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = 1});
							break;
						default:
							// no up here
							break;
					}
				}
				// get rid of 'start' square for easier calculations
				switch (dirs) {
					case 0x3:
						// NS
						grid[idx].p = P_VERT;
						break;
					case 0xC:
						// EW
						grid[idx].p = P_HORI;
						break;
					case 0x9:
						// NE
						grid[idx].p = P_NE90;
						break;
					case 0x5:
						// NW
						grid[idx].p = P_NW90;
						break;
					case 0x6:
						// SW
						grid[idx].p = P_SW90;
						break;
					case 0xA:
						// SE
						grid[idx].p = P_SE90;
						break;
				}
			} break;
			case P_HORI: {
				enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_VERT: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = curr.dist + 1});
			} break;
			case P_NE90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_NW90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_SE90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			case P_SW90: {
				enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1, .dist = curr.dist + 1});
				enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y, .dist = curr.dist + 1});
			} break;
			default:
				printf("%c\n", chr_to_pipe[grid[idx].p]);
				assert(false);
		}
	}

	subtile* subgrid = malloc(w * h * 9 * sizeof(subtile));

	// we have marked all of the tiles that are part of the main loop
	// let's upscale those
	for (size_t y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++) {
			if (grid[x + y * w].dist == -1) {
				// unimportant
				// just fill with 'inside's
				for (size_t sx = x * 3; sx < x * 3 + 3; sx++) {
					for (size_t sy = y * 3; sy < y*3+3; sy++) {
						subgrid[sx + sy * w * 3] = INSIDE;
					}
				}
			} else {
				// fill with 'inside's then add the actual wall
				for (size_t sx = x * 3; sx < x * 3 + 3; sx++) {
					for (size_t sy = y * 3; sy < y*3+3; sy++) {
						subgrid[sx + sy * w * 3] = INSIDE;
					}
				}
				size_t sx = x * 3 + 1;
				size_t sy = y * 3 + 1;
				switch (grid[x + y * w].p) {
					case P_HORI:
						subgrid[(sx + 1) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy) * w*3] = WALL;
						subgrid[(sx - 1) + (sy) * w*3] = WALL;
						break;
					case P_VERT:
						subgrid[(sx) + (sy + 1) * w*3] = WALL;
						subgrid[(sx) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy - 1) * w*3] = WALL;
						break;
					case P_NE90:
						subgrid[(sx + 1) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy - 1) * w*3] = WALL;
						break;
					case P_NW90:
						subgrid[(sx - 1) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy - 1) * w*3] = WALL;
						break;
					case P_SE90:
						subgrid[(sx + 1) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy + 1) * w*3] = WALL;
						break;
					case P_SW90:
						subgrid[(sx - 1) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy) * w*3] = WALL;
						subgrid[(sx) + (sy + 1) * w*3] = WALL;
						break;
					default:
						printf("%c\n", chr_to_pipe[grid[x + y * w].p]);
						assert(false);
				}
			}
		}
	}

	// reset the queue
	queue.start = 0;
	queue.end = 0;
	start.x = 0;
	start.y = 0;
	enqueue(&queue, start);
	while (has_data(queue)) {
		ivec3 curr = dequeue(&queue);
		if (curr.x >= w * 3 || curr.y >= h * 3) {
			// out of bounds, skip
			continue;
		}
		// is this an allowed entrance?
		size_t idx = curr.x + curr.y * w * 3;
		// skip this one
		if (subgrid[idx] != INSIDE) continue;
		subgrid[idx] = OUTSIDE;
		enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y - 1});
		enqueue(&queue, (ivec3){.x = curr.x + 1, .y = curr.y});
		enqueue(&queue, (ivec3){.x = curr.x, .y = curr.y + 1});
		enqueue(&queue, (ivec3){.x = curr.x - 1, .y = curr.y});
	}

	size_t cnt = 0;
	for (size_t y = 0; y < h; y++) {
		for (size_t x = 0; x < w; x++) {
			size_t sx = x * 3 + 1, sy = y * 3 + 1;
			if (subgrid[sx + sy * w * 3] == INSIDE) cnt++;
		}
	}

	printf("%lu\n", cnt);

	free(grid);
	free(subgrid);
	free(queue.data);
}
