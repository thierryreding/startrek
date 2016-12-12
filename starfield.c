#include <errno.h>
#include <stdlib.h>
#include <stdio.h> /* XXX */

#include <drm-kms.h>

#include "starfield.h"

int starfield_create(struct starfield **starfieldp,
		     const struct viewport *viewport)
{
	const unsigned int layers = 5;
	unsigned int i, width, height;
	struct starfield *starfield;
	unsigned int count;
	struct star *star;

	width = viewport->ex - viewport->sx + 1;
	height = viewport->ey - viewport->sy + 1;
	count = (width * height) / 750;

	starfield = malloc(sizeof(*starfield));
	if (!starfield)
		return -ENOMEM;

	starfield->viewport = *viewport;
	starfield->num_stars = count;

	starfield->stars = calloc(count, sizeof(*star));
	if (!starfield) {
		free(starfield);
		return -ENOMEM;
	}

	for (i = 0; i < count; i++) {
		star = &starfield->stars[i];

		star->x = viewport->sx + rand() % width;
		star->y = viewport->sy + rand() % height;
		star->color = rand() % layers;
	}

	*starfieldp = starfield;

	return 0;
}

void starfield_free(struct starfield *starfield)
{
	if (!starfield)
		return;

	free(starfield->stars);
	free(starfield);
}

void starfield_move(struct starfield *starfield)
{
	unsigned int i;

	for (i = 0; i < starfield->num_stars; i++) {
		struct star *star = &starfield->stars[i];
		/* XXX vary speed to make starfield look more dynamic */
		unsigned int speed = star->color;

		if (star->x < speed)
			star->x += starfield->viewport.ex;

		star->x -= speed;
	}
}

void starfield_put(struct starfield *starfield, struct surface *surface)
{
	uint8_t *fb = surface->buffer;
	unsigned int i;

	for (i = 0; i < starfield->num_stars; i++) {
		const struct star *star = &starfield->stars[i];

		fb[star->y * surface->pitch + star->x] = star->color;
	}
}
