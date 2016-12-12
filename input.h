#ifndef INPUT_H
#define INPUT_H

#include <libinput.h>
#include <libudev.h>

#include "common.h"
#include "events.h"

struct input {
	struct event_source source;
	struct libinput *input;
	struct udev *udev;
};

int input_create(struct input **inputp);
void input_free(struct input *input);
int input_poll(struct input *input, struct event_loop *loop);

#endif /* INPUT_H */
