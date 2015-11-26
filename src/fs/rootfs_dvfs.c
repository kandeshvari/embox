/**
 * @file
 * @details
 *
 * @date 2 Apr 2015
 * @author Denis Deryugin
 */

#include <errno.h>
#include <string.h>

#include <fs/dvfs.h>
#include <drivers/block_dev.h>
#include <drivers/char_dev.h>
#include <embox/unit.h>
#include <util/array.h>

EMBOX_UNIT_INIT(rootfs_mount);

ARRAY_SPREAD_DEF(struct auto_mount *, auto_mount_tab);

static struct super_block *root_sb;

extern int dvfs_update_root(void);

int set_rootfs_sb(struct super_block *sb) {
	root_sb = sb;
	return 0;
}

struct super_block *rootfs_sb(void) {
	return root_sb;
}

/* @brief Perform mount of the root file system
 * @param dev     Path to the block device (e.g. /dev/sda1)
 * @param fs_type Name of the file system driver
 *
 * @return Negative error number
 * @retval       0 Ok
 * @revtal -ENOENT File system driver not found
 */
static int rootfs_mount(void) {
	const char *dev, *fs_type;
	struct dumb_fs_driver *fsdrv;
	struct auto_mount *auto_mnt;
	struct lookup lu;
	char *tmp;

	dev = OPTION_STRING_GET(bdev);
	fs_type = OPTION_STRING_GET(fstype);
	assert(fs_type);

	fsdrv = dumb_fs_driver_find(fs_type);
	if (fsdrv == NULL)
		return -ENOENT;

	dvfs_update_root();

	if (-1 == dvfs_mount(dev, "/", (char *) fs_type, 0))
		return -errno;

	array_spread_foreach(auto_mnt, auto_mount_tab) {
		if (fsdrv == auto_mnt->fs_driver)
			continue;

		dvfs_lookup(auto_mnt->mount_path, &lu);
		if (lu.item == NULL) {
			tmp = strrchr(auto_mnt->mount_path, '/');
			dvfs_create_new(tmp ? tmp + 1: auto_mnt->mount_path,
					&lu, DVFS_DIR_VIRTUAL | S_IFDIR);
		}
		dvfs_mount(NULL, auto_mnt->mount_path, auto_mnt->fs_driver->name, 0);
	}

	return 0;
}
