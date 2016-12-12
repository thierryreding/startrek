#ifndef EVENTS_H
#define EVENTS_H

#include <poll.h>

#include "list.h"

#define EVENT_SOURCE_INPUT  (1 << 0)
#define EVENT_SOURCE_OUTPUT (1 << 1)

struct event_source {
	struct list_head list;
	struct pollfd *poll;
	unsigned long flags;
	int fd;

	int (*handle)(struct event_source *source);
};

struct event_loop {
	unsigned int num_fds;
	struct pollfd *fds;

	struct list_head sources;
	unsigned int num_sources;
};

int event_loop_create(struct event_loop **loopp);
void event_loop_free(struct event_loop *loop);

int event_loop_add(struct event_loop *loop, struct event_source *source);
int event_loop_poll(struct event_loop *loop);

#endif /* EVENTS_H */
