/*
 * enumerate-devices.c
 *
 * Copyright (c) 2017 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 3.
 */

#include <stdio.h>
#include <stdlib.h>

#include <libudev.h>

#include <string.h>
#include <errno.h>

static void dump_entries(struct udev_device *dev,
                         struct udev_list_entry *entries,
                         const char *const type)
{
    struct udev_list_entry *entry;
    const char *name;
    const char *value;

    udev_list_entry_foreach(entry, entries) {
        name = udev_list_entry_get_name(entry);

        if (type != NULL) {
            if (strcmp(type, "prop") == 0)
                value = udev_device_get_property_value(dev, name);
            else if (strcmp(type, "sysattr") == 0)
                value = udev_device_get_sysattr_value(dev, name);
        } else {
            value = udev_list_entry_get_value(entry);
        }

        if (value == NULL)
            printf("    %s\n", name);
        else
            printf("    %s: %s\n", name, value);
    }
}

static void dump_properties(struct udev *ctx, const char *const syspath)
{
    struct udev_device *dev;
    struct udev_list_entry *entries;

    dev = udev_device_new_from_syspath(ctx, syspath);
    printf("  %s\n", udev_device_get_devpath(dev));
    printf("  %s\n", udev_device_get_subsystem(dev));
    printf("  %s\n", udev_device_get_devtype(dev));
    printf("  %s\n", udev_device_get_syspath(dev));
    printf("  %s\n", udev_device_get_sysname(dev));
    printf("  %s\n", udev_device_get_sysnum(dev));
    printf("  %s\n", udev_device_get_devnode(dev));
    printf("  %d\n", udev_device_get_is_initialized(dev));

    entries = udev_device_get_devlinks_list_entry(dev);
    if (entries != NULL) {
        printf("  devlinks:\n");
        dump_entries(dev, entries, NULL);
    }

    entries = udev_device_get_properties_list_entry(dev);
    if (entries != NULL) {
        printf("  properties:\n");
        dump_entries(dev, entries, "prop");
    }

    entries = udev_device_get_tags_list_entry(dev);
    if (entries != NULL) {
        printf("  tags:\n");
        dump_entries(dev, entries, NULL);
    }

    entries = udev_device_get_sysattr_list_entry(dev);
    if (entries != NULL) {
        printf("  sysattr:\n");
        dump_entries(dev, entries, "sysattr");
    }
}

static void enumerate_device(struct udev *ctx, const char *const subsystem)
{
    struct udev_enumerate *enumerator;
    struct udev_list_entry *entry, *entries;
    const char *name;
    const char *value;
    int err;

    enumerator = udev_enumerate_new(ctx);

    if (udev_enumerate_add_match_subsystem(enumerator, subsystem) < 0) {
        printf("udev_enumerate_add_match_subsystem(3): %s\n", strerror(ENXIO));
        return;
    }

    err = udev_enumerate_scan_devices(enumerator);
    if (err < 0) {
        printf("udev_enumerate_scan_devices(3): %s\n", strerror(err));
        return;
    }

    entries = udev_enumerate_get_list_entry(enumerator);
    if (entries == NULL) {
        printf("udev_enumerate_get_list_entry(3): %s\n", strerror(ENOMEM));
        return;
    }

    udev_list_entry_foreach(entry, entries) {
        name = udev_list_entry_get_name(entry);
        if (name == NULL)
                continue;
        printf("%s\n", name);
        dump_properties(ctx, name);
        printf("\n");
    }

    udev_enumerate_unref(enumerator);
}

int main(int argc, const char *const argv[])
{
    struct udev *ctx;
    struct udev_device *dev;
    const char *subsystem;

    if (argc == 1)
        subsystem = NULL;
    else
        subsystem = argv[1];

    ctx = udev_new();
    if (ctx == NULL) {
        printf("udev_new(3): %s\n", strerror(ENOMEM));
        return EXIT_FAILURE;
    }

    enumerate_device(ctx, subsystem);

    return EXIT_SUCCESS;
}
