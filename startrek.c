#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "events.h"
#include "input.h"
#include "list.h"
#include "screen.h"
#include "sprite.h"
#include "starfield.h"

static const char DEFAULT_DEVICE[] = "/dev/dri/card0";

static bool done = false;

static void signal_handler(int signum)
{
	if (signum == SIGINT)
		done = true;
}

static void usage(FILE *fp, const char *program)
{
	fprintf(fp, "usage: %s [options] DEVICE\n", program);
	fprintf(fp, "\n");
	fprintf(fp, "options:\n");
	fprintf(fp, "  -h, --help	display this help screen and exit\n");
	fprintf(fp, "\n");
}

static const uint8_t enterprise_data[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0,
	0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 0, 0,
	5, 5, 5, 5, 5, 5, 6, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 0,
	0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 5, 5, 5, 5,
	0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 5, 5, 5, 5,
	0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 6, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 0,
	0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0,
};

static const struct sprite enterprise = {
	.width = 20,
	.height = 10,
	.data = enterprise_data,
};

static const uint8_t klingon_cruiser_data[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0,
	0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 5, 5, 0, 0, 0,
	7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0,
	7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0,
	0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 5, 5, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0,
};

static const struct sprite klingon_cruiser = {
	.width = 20,
	.height = 10,
	.data = klingon_cruiser_data,
};

int main(int argc, char *argv[])
{
	static const struct option options[] = {
		{ "help", 0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};
	static const char opts[] = "h";
	struct starfield *starfield;
	struct event_loop *loop;
	struct screen *screen;
	struct input *input;
	struct sigaction sa;
	const char *device;
	bool help = false;
	int err, opt;

	while ((opt = getopt_long(argc, argv, opts, options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			help = true;
			break;

		default:
			usage(stderr, argv[0]);
			return 1;
		}
	}

	if (help) {
		usage(stdout, argv[0]);
		return 0;
	}

	if (optind >= argc)
		device = DEFAULT_DEVICE;
	else
		device = argv[optind];

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;

	err = sigaction(SIGINT, &sa, NULL);
	if (err < 0) {
		fprintf(stderr, "failed to setup signal handler: %m\n");
		return 1;
	}

	err = event_loop_create(&loop);
	if (err < 0) {
		fprintf(stderr, "failed to setup event loop: %s\n", strerror(-err));
		return 1;
	}

	err = screen_create(&screen, device);
	if (err < 0) {
		fprintf(stderr, "failed to create screen: %s\n", strerror(-err));
		return 1;
	}

	err = screen_poll(screen, loop);
	if (err < 0) {
		fprintf(stderr, "failed to add screen to event loop: %s\n", strerror(-err));
		return 1;
	}

	err = input_create(&input);
	if (err < 0) {
		fprintf(stderr, "failed to setup input: %s\n", strerror(-err));
		return 1;
	}

	err = input_poll(input, loop);
	if (err < 0) {
		fprintf(stderr, "failed to add input to event loop: %s\n", strerror(-err));
		return 1;
	}

	err = starfield_create(&starfield, &screen->viewport);
	if (err < 0) {
		fprintf(stderr, "failed to create starfield: %s\n", strerror(-err));
		return 1;
	}

	while (!done) {
		struct surface *fb;

		err = screen_lock(screen, &fb);
		if (err < 0) {
			fprintf(stderr, "failed to lock surface: %d\n", err);
			break;
		}

		starfield_move(starfield);

		memset(fb->buffer, 0, fb->size);

		starfield_put(starfield, fb);
		sprite_put(&enterprise, fb, 0, 0);
		sprite_put(&klingon_cruiser, fb, 0, 32);

		screen_unlock(screen, fb);
		screen_flip(screen);

		err = event_loop_poll(loop);
		if (err < 0)
			break;
	}

	starfield_free(starfield);
	screen_free(screen);

	return 0;
}
