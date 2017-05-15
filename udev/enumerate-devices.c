#include <stdio.h>
#include <stdlib.h>

#include <libudev.h>

static void dump_tags(struct udev_device *dev)
{
    struct udev_list_entry *entry, *entries;

    entries = udev_device_get_tags_list_entry(dev);

    udev_list_entry_foreach(entry, entries) {
        const char *name = udev_list_entry_get_name(entry);
        printf("    = %s\n", name);
    }
}

static void dump_sysattrs(struct udev_device *dev)
{
    struct udev_list_entry *entry, *entries;

    entries = udev_device_get_sysattr_list_entry(dev);

    udev_list_entry_foreach(entry, entries) {
        const char *name = udev_list_entry_get_name(entry);
        printf("    * %s: %s\n", name, udev_device_get_sysattr_value(dev, name));
    }
}

static void dump_props(struct udev_device *dev)
{
    struct udev_list_entry *entry, *entries;

    entries = udev_device_get_properties_list_entry(dev);

    udev_list_entry_foreach(entry, entries) {
        const char *name = udev_list_entry_get_name(entry);
        printf("    - %s: %s\n", name, udev_device_get_property_value(dev, name));
    }
}

int main(int argc, const char *argv[])
{
    struct udev *ctx;

    ctx = udev_new();
    if (ctx == NULL)
        return EXIT_FAILURE;

    struct udev_enumerate *iter;
    iter = udev_enumerate_new(ctx);
    if (iter == NULL)
        goto end;

    udev_enumerate_add_match_subsystem(iter, "sound");
    udev_enumerate_scan_devices(iter);

    struct udev_list_entry *entry, *entries;
    entries = udev_enumerate_get_list_entry(iter);
    udev_list_entry_foreach(entry, entries) {
        const char *syspath;
        struct udev_device *dev;
        syspath = udev_list_entry_get_name(entry);
        dev = udev_device_new_from_syspath(ctx, syspath);
        printf("%s\n", syspath);

        struct udev_device *parent;
        parent = udev_device_get_parent(dev);

        syspath = udev_device_get_syspath(parent);
        if (syspath == NULL)
            continue;
        printf("  %s\n", syspath);

        parent = udev_device_get_parent_with_subsystem_devtype(dev, "firewire", NULL);
        syspath = udev_device_get_syspath(parent);
        if (syspath == NULL)
            continue;
        printf("  %s %s\n", syspath, udev_device_get_sysname(parent));
        dump_tags(parent);
        dump_props(parent);
        dump_sysattrs(parent);
        /*
        printf("%s %s %s %s %s %s %s\n",
        udev_device_get_devpath(parent),
        udev_device_get_subsystem(parent),
        udev_device_get_devtype(parent),
        udev_device_get_syspath(parent),
        udev_device_get_sysname(parent),
        udev_device_get_sysnum(parent),
        udev_device_get_devnode(parent)
        );
        */


        parent = udev_device_get_parent_with_subsystem_devtype(parent, "firewire", NULL);
        syspath = udev_device_get_syspath(parent);
        if (syspath == NULL)
            continue;
        printf("  %s %s\n", syspath, udev_device_get_sysname(parent));
        dump_tags(parent);
        dump_props(parent);
        dump_sysattrs(parent);
        /*
        printf("%s %s %s %s %s %s %s\n",
        udev_device_get_devpath(parent),
        udev_device_get_subsystem(parent),
        udev_device_get_devtype(parent),
        udev_device_get_syspath(parent),
        udev_device_get_sysname(parent),
        udev_device_get_sysnum(parent),
        udev_device_get_devnode(parent)
        );
        */
    }
end:
    udev_unref(ctx);
    return EXIT_SUCCESS;
}
