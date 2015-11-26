/**
 * @file
 * @brief
 *
 * @date 29.10.2012
 * @author Andrey Gazukin
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mem/misc/pool.h>

#include <fs/mbr.h>
#include <drivers/block_dev.h>
#include <drivers/block_dev/partition.h>
#include <framework/mod/options.h>

/* TODO Create Partition as drive */
int create_partitions(struct block_dev *bdev) {
	struct mbr mbr;
	int rc;
	int part_n;
	char part_name[0x16];
	struct block_dev *part_bdev;

	/* Read partition table */
	rc = block_dev_read(bdev, (char *)&mbr, bdev->block_size, 0);
	if (rc < 0) {
		return rc;
	}

	/* Create partition devices */
	if ((mbr.sig_55 != 0x55) || (mbr.sig_aa != 0xAA)) {
		return 0;
	}
	for (part_n = 0; part_n < 4; part_n ++) {

		if(mbr.ptable[part_n].type == 0) {
			return part_n;
		}

		sprintf(part_name, "%s/%s%d", "/dev/", bdev->name, part_n + 1);
		part_bdev = block_dev_create(part_name, bdev->driver, NULL);
		if (!part_bdev) {
			return -ENOMEM;
		}

		part_bdev->start_offset = (uint32_t)(mbr.ptable[part_n].start_3) << 24 |
				(uint32_t)(mbr.ptable[part_n].start_2) << 16 |
				(uint32_t)(mbr.ptable[part_n].start_1) << 8 |
				(uint32_t)(mbr.ptable[part_n].start_0) << 0;

		part_bdev->size = (uint32_t)(mbr.ptable[part_n].size_3) << 24 |
				(uint32_t)(mbr.ptable[part_n].size_2) << 16 |
				(uint32_t)(mbr.ptable[part_n].size_1) << 8 |
				(uint32_t)(mbr.ptable[part_n].size_0) << 0;

		part_bdev->parrent_bdev = bdev;
		part_bdev->block_size = bdev->block_size;
	}

	return 0;
}
