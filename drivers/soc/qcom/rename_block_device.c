// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/genhd.h>
#include <linux/device.h>
#include <linux/of.h>
#define PATH_SIZE	32

static int rename_blk_dev_init(void)
{
	dev_t dev;
	int index = 0, partno;
	struct gendisk *disk;
	struct device_node *node;
	char dev_path[PATH_SIZE];
	const char *actual_name, *modified_name;

	node = of_find_compatible_node(NULL, NULL, "qcom,blkdev-rename");
	if (!node) {
		pr_err("qcom,blkdev-rename is missing\n");
		goto out;
	}
	while (!of_property_read_string_index(node, "actual-dev", index,
						&actual_name)) {
		memset(dev_path, '\0', PATH_SIZE);
		snprintf(dev_path, PATH_SIZE, "/dev/%s", actual_name);
		dev = name_to_dev_t(dev_path);
		if (!dev) {
			pr_err("No device path : %s\n", dev_path);
			return -EINVAL;
		}
		disk = get_gendisk(dev, &partno);
		if (!disk) {
			pr_err("No device with dev path : %s\n", dev_path);
			return -ENXIO;
		}
		if (!of_property_read_string_index(node, "rename-dev", index,
							&modified_name)) {
			device_rename(disk_to_dev(disk), modified_name);
		} else {
			pr_err("rename-dev for actual-dev = %s is missing\n",
								 actual_name);
			return -ENXIO;
		}
		index++;
	}
out:
	return  0;
}

late_initcall(rename_blk_dev_init);
MODULE_DESCRIPTION("Rename block devices");
MODULE_LICENSE("GPL v2");
