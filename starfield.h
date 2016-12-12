#ifndef STARFIELD_H
#define STARFIELD_H

#include "common.h"
#include "screen.h"

struct star {
	unsigned int x;
	unsigned int y;
	uint8_t color;
};

struct starfield {
	struct viewport viewport;
	unsigned int num_stars;
	struct star *stars;
};

int starfield_create(struct starfield **starfieldp,
		     const struct viewport *viewport);
void starfield_free(struct starfield *starfield);
void starfield_move(struct starfield *starfield);
void starfield_put(struct starfield *starfield, struct surface *surface);

#endif /* STARFIELD_H */
