#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <drm_fourcc.h>

#include "screen.h"

int screen_create(struct screen **screenp, const char *device)
{
	struct drm_kms_screen_args args;
	struct drm_kms_lut *lut;
	struct screen *screen;
	int err;

	screen = calloc(1, sizeof(*screen));
	if (!screen)
		return -ENOMEM;

	screen->fd = open(device, O_RDWR);
	if (screen->fd < 0) {
		err = -errno;
		goto free;
	}

	memset(&args, 0, sizeof(args));
	args.flags = DRM_KMS_SCREEN_FULLSCREEN;
	args.format = DRM_FORMAT_C8;

	err = drm_kms_screen_create_with_args(&screen->screen, screen->fd,
					      &args);
	if (err < 0) {
		fprintf(stderr, "failed to create screen: %s\n", strerror(-err));
		goto close;
	}

	err = drm_kms_lut_load_palette(&lut, "data/startrek.pal");
	if (err < 0) {
		fprintf(stderr, "failed to read palette: %s\n", strerror(-err));
		goto free_screen;
	}

	err = drm_kms_screen_load_lut(screen->screen, lut);
	if (err < 0) {
		fprintf(stderr, "failed to load palette: %s\n", strerror(-err));
		goto free_lut;
	}

	drm_kms_lut_free(lut);

	screen->viewport.sx = screen->viewport.sy = 0;
	screen->viewport.ex = screen->screen->width - 1;
	screen->viewport.ey = screen->screen->height - 1;

	*screenp = screen;

	return 0;

free_lut:
	drm_kms_lut_free(lut);
free_screen:
	drm_kms_screen_free(screen->screen);
close:
	close(screen->fd);
free:
	free(screen);
	return err;
}

void screen_free(struct screen *screen)
{
	if (!screen)
		return;

	drm_kms_screen_free(screen->screen);
	close(screen->fd);
	free(screen);
}

static void screen_handle_page_flip(int fd, unsigned int frame,
				    unsigned int sec, unsigned int usec,
				    void *data)
{
}

static int screen_handle_events(struct event_source *source)
{
	struct screen *screen = container_of(source, struct screen, source);

	drmHandleEvent(screen->fd, &screen->evctx);

	return 0;
}

int screen_poll(struct screen *screen, struct event_loop *loop)
{
	int err;

	screen->evctx.version = DRM_EVENT_CONTEXT_VERSION;
	screen->evctx.vblank_handler = NULL;
	screen->evctx.page_flip_handler = screen_handle_page_flip;

	screen->source.fd = screen->fd;
	screen->source.flags = EVENT_SOURCE_INPUT;
	screen->source.handle = screen_handle_events;

	err = event_loop_add(loop, &screen->source);
	if (err < 0)
		return err;

	return 0;
}

int screen_lock(struct screen *screen, struct surface **surfacep)
{
	struct drm_kms_screen *s = screen->screen;
	struct drm_kms_surface *fb = s->fb[s->current];
	struct surface *surface;
	void *buffer;
	int err;

	err = drm_kms_surface_lock(fb, &buffer);
	if (err < 0)
		return err;

	surface = calloc(1, sizeof(*surface));
	if (!surface) {
		drm_kms_surface_unlock(fb);
		return -ENOMEM;
	}

	surface->width = fb->width;
	surface->height = fb->height;
	surface->pitch = fb->bo->pitch;
	surface->size = fb->bo->size;
	surface->buffer = buffer;

	*surfacep = surface;

	return 0;
}

void screen_unlock(struct screen *screen, struct surface *surface)
{
	struct drm_kms_screen *s = screen->screen;
	struct drm_kms_surface *fb = s->fb[s->current];

	drm_kms_surface_unlock(fb);

	free(surface);
}

void screen_flip(struct screen *screen)
{
	drm_kms_screen_flip(screen->screen, screen);
}
