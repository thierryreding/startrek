#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "input.h"

static int open_restricted(const char *path, int flags, void *user_data)
{
	int fd;

	fd = open(path, flags);
	if (fd < 0)
		return -errno;

	return fd;
}

static void close_restricted(int fd, void *user_data)
{
	close(fd);
}

static const struct libinput_interface interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

int input_create(struct input **inputp)
{
	struct input *input;
	int err = 0;

	input = malloc(sizeof(*input));
	if (!input)
		return -ENOMEM;

	input->udev = udev_new();
	if (!input->udev) {
		err = -ENOMEM;
		goto free;
	}

	input->input = libinput_udev_create_context(&interface, NULL,
						    input->udev);
	if (!input->input) {
		err = -ENOMEM;
		goto unref;
	}

	err = libinput_udev_assign_seat(input->input, "seat0");
	if (err < 0)
		goto unref;

	*inputp = input;

	return 0;

unref:
	libinput_unref(input->input);
	udev_unref(input->udev);
free:
	free(input);
	return err;
}

void input_free(struct input *input)
{
	if (!input)
		return;

	free(input);
}

static int input_handle_events(struct event_source *source)
{
	struct input *input = container_of(source, struct input, source);
	struct libinput_device *device;
	struct libinput_event *event;
	const char *name;

	libinput_dispatch(input->input);

	while ((event = libinput_get_event(input->input)) != NULL) {
		device = libinput_event_get_device(event);
		name = libinput_device_get_name(device);
		printf("device: %s\n", name);
	}

	return 0;
}

int input_poll(struct input *input, struct event_loop *loop)
{
	int err;

	input->source.fd = libinput_get_fd(input->input);
	input->source.flags = EVENT_SOURCE_INPUT;
	input->source.handle = input_handle_events;

	err = event_loop_add(loop, &input->source);
	if (err < 0)
		return err;

	return 0;
}
