#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define container_of(ptr, type, member) ({				\
		const typeof(((type *)0)->member) *__mptr = (ptr);	\
		(type *)((char *)__mptr - offsetof(type, member));	\
	})

struct viewport {
	unsigned int sx;
	unsigned int sy;
	unsigned int ex;
	unsigned int ey;
};

#endif /* COMMON_H */
