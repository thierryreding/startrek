#include <errno.h>
#include <stdio.h> /* XXX */
#include <stdlib.h>

#include "events.h"

int event_loop_create(struct event_loop **loopp)
{
	struct event_loop *loop;

	loop = calloc(1, sizeof(*loop));
	if (!loop)
		return -ENOMEM;

	INIT_LIST_HEAD(&loop->sources);

	*loopp = loop;
	return 0;
}

void event_loop_free(struct event_loop *loop)
{
	if (!loop)
		return;

	free(loop->fds);
	free(loop);
}

int event_loop_add(struct event_loop *loop, struct event_source *source)
{
	unsigned int num_fds = loop->num_fds + 1;
	struct pollfd *fds;

	fds = realloc(loop->fds, sizeof(*fds) * num_fds);
	if (!fds)
		return -ENOMEM;

	loop->num_fds = num_fds;
	loop->fds = fds;

	list_add_tail(&source->list, &loop->sources);
	source->poll = &loop->fds[num_fds - 1];

	source->poll->fd = source->fd;
	source->poll->revents = 0;
	source->poll->events = 0;

	if (source->flags & EVENT_SOURCE_OUTPUT)
		source->poll->events |= POLLOUT;

	if (source->flags & EVENT_SOURCE_INPUT)
		source->poll->events |= POLLIN;

	loop->num_sources++;

	return 0;
}

int event_loop_poll(struct event_loop *loop)
{
	struct event_source *source;
	int err;

	err = poll(loop->fds, loop->num_fds, -1);
	if (err <= 0) {
		if (err == 0)
			return -ETIMEDOUT;

		return -errno;
	}

	list_for_each_entry(source, &loop->sources, list) {
		if (source->flags & EVENT_SOURCE_INPUT) {
			if (source->poll->events & POLLIN) {
				err = source->handle(source);
				if (err < 0) {
					fprintf(stderr, "failed to handle input events for %p\n", source);
				}
			}
		}

		if (source->flags & EVENT_SOURCE_OUTPUT) {
			if (source->poll->events & POLLOUT) {
				err = source->handle(source);
				if (err < 0) {
					fprintf(stderr, "failed to handle output events for %p\n", source);
				}
			}
		}
	}

	return 0;
}
