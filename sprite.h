#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

#include "screen.h"

struct sprite {
	unsigned int width;
	unsigned int height;
	const uint8_t *data;
};

void sprite_put(const struct sprite *sprite, struct surface *surface,
		unsigned int x, unsigned int y);

#endif /* SPRITE_H */
