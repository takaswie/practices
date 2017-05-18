/*
 * listen-events.c
 *
 * Copyright (c) 2017 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 3.
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <libudev.h>

#include <string.h>
#include <errno.h>

#include <sys/epoll.h>

static void dump_event(struct udev_monitor *mon)
{
    struct udev_device *device;

    device = udev_monitor_receive_device(mon);
    printf("%s: %s\n",
           udev_device_get_subsystem(device), udev_device_get_syspath(device));
}

static int loop_io(struct udev_monitor *mon)
{
    int epfd;
    struct epoll_event ev = {0};

    int fd;

    int count;
    int err;

    epfd = epoll_create1(0);
    if (epfd < 0) {
        printf("epoll_create(2): %s\n", strerror(errno));
        return -errno;
    }

    fd = udev_monitor_get_fd(mon);
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        printf("epoll_ctl(2): %s\n", strerror(errno));
        err = -errno;
        goto end;
    }

    udev_monitor_enable_receiving(mon);

    while (1) {
        memset(&ev, 0, sizeof(struct epoll_event));
        count = epoll_wait(epfd, &ev, 1, 200);
        if(count < 0) {
            if (errno == EINTR)
                continue;
            printf("epoll_wait(2): %s\n", strerror(errno));
            err = -errno;
            break;
        }
        if (count > 0)
            dump_event(mon);
    }
end:
    close(epfd);
    return err;
}

int main(int argc, const char *argv[])
{
    const char *group;
    struct udev *ctx;
    struct udev_monitor *mon;
    int err;

    if (argc == 2)
        group = "kernel";
    else
        group = "udev";

    ctx = udev_new();
    if (ctx == NULL) {
        printf("udev_new()\n");
        err = -1;
        goto end_err;
    }

    mon = udev_monitor_new_from_netlink(ctx, group);
    if (mon == NULL) {
        printf("udev_monitor_new_from_netlink()\n");
        err = -1;
        goto end_ctx;
    }
    printf("group: %s\n", group);

    err = loop_io(mon);

    udev_monitor_unref(mon);

end_ctx:
    udev_unref(ctx);
end_err:
    if (err < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
