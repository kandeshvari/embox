/**
 * @file
 *
 * @data 23.11.2015
 * @author: Anton Bondarev
 */

#ifndef SRC_FS_FUSE_FUSE_MODULE_H_
#define SRC_FS_FUSE_FUSE_MODULE_H_


struct fuse_module {
	const char *fuse_module_name;
	const char *fuse_module_cmd_mount;
};

extern struct fuse_module *fuse_module_lookup(char *fuse_type);

extern int fuse_module_mount(struct fuse_module *fm, char *dev, char *dest);

extern int fuse_module_umount(struct fuse_module *fm);

#include <util/array.h>
#define FUSE_MODULE_DEF(name, cmd) \
	static const struct fuse_module fm##name = {name, cmd}; \
	ARRAY_SPREAD_DECLARE(struct fuse_module *, fuse_module_repo); \
	ARRAY_SPREAD_ADD( fuse_module_repo, &fm##name)

#endif /* SRC_FS_FUSE_FUSE_MODULE_H_ */
