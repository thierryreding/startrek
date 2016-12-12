#ifndef SCREEN_H
#define SCREEN_H

#include <drm-kms.h>

#include "events.h"

struct surface {
	unsigned int width;
	unsigned int height;
	void *buffer;
	size_t pitch;
	size_t size;
};

struct screen {
	struct event_source source;
	struct drm_kms_screen *screen;
	struct viewport viewport;
	drmEventContext evctx;
	int fd;
};

int screen_create(struct screen **screenp, const char *device);
void screen_free(struct screen *screen);
int screen_poll(struct screen *screen, struct event_loop *loop);

int screen_lock(struct screen *screen, struct surface **surfacep);
void screen_unlock(struct screen *screen, struct surface *surface);
void screen_flip(struct screen *screen);

#endif /* SCREEN_H */
