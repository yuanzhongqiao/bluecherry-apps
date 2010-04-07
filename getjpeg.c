/*
 * Copyright (C) 2010 Bluecherry, LLC
 *
 * Confidential, all rights reserved. No distribution is permitted.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <errno.h>

#include <libbluecherry.h>

static void print_error(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fprintf(stderr, "\n");

	exit(1);
}

static void print_image(struct bc_handle *bc)
{
	bc_buf_get(bc);

	if (fwrite(bc_buf_data(bc), bc_buf_size(bc), 1, stdout) != 1)
		print_error("Error writing jpeg: %m");
}

extern char *__progname;

static void usage(void)
{
        fprintf(stderr, "Usage: %s <args> > outfile\n", __progname);
	fprintf(stderr, "  -c\tDevice channel (default 0)\n");
	exit(1);
}

int main(int argc, char **argv)
{
	struct bc_handle *bc;
	char dev[256];
	int ch = 0;
	int opt;

	while ((opt = getopt(argc, argv, "c:h")) != -1) {
		switch (opt) {
		case 'c': ch = atoi(optarg); break;
		case 'h': default: usage();
		}
	}

	if (ch < 0 || ch > 15)
		print_error("Invalid channel %d", ch);

	/* Setup the device */
	sprintf(dev, "/dev/video%d", ch + 1);
	if ((bc = bc_handle_get(dev)) == NULL)
		print_error("%s: error opening device: %m", dev);

	/* Setup for MJPEG, leave everything else as default */
	bc->vfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	if (ioctl(bc->dev_fd, VIDIOC_S_FMT, &bc->vfmt) < 0)
		print_error("%s: error setting mjpeg: %m", dev);

	if (bc_handle_start(bc))
		print_error("%s: error starting stream: %m", dev);

	print_image(bc);

	bc_handle_free(bc);

	exit(0);
}
