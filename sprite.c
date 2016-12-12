#include "sprite.h"

void sprite_put(const struct sprite *sprite, struct surface *surface,
		unsigned int x, unsigned int y)
{
	uint8_t *fb = surface->buffer;
	unsigned int i, j;

	for (j = 0; j < sprite->height; j++) {
		unsigned int offset = (y + j) * surface->pitch + x;

		for (i = 0; i < sprite->width; i++)
			fb[offset + i] = sprite->data[j * sprite->width + i];
	}
}
