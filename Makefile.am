ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = startrek

startrek_CFLAGS = \
	@LIBUDEV_CFLAGS@ \
	@LIBINPUT_CFLAGS@ \
	@DRM_KMS_CFLAGS@ \
	@LIBDRM_CFLAGS@

startrek_SOURCES = \
	common.h \
	events.c \
	events.h \
	input.c \
	input.h \
	list.h \
	screen.c \
	screen.h \
	sprite.c \
	sprite.h \
	starfield.c \
	starfield.h \
	startrek.c

startrek_LDADD = \
	@LIBUDEV_LIBS@ \
	@LIBINPUT_LIBS@ \
	@DRM_KMS_LIBS@ \
	@LIBDRM_LIBS@
