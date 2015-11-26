/**
 * @file
 * @brief Implementation of FAT driver for DVFS
 *
 * @date   11 Apr 2015
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>

#include <drivers/device.h>
#include <fs/fat.h>
#include <fs/dvfs.h>
#include <util/math.h>

extern uint8_t fat_sector_buff[FAT_MAX_SECTOR_SIZE];

extern struct file_operations fat_fops;

int fat_read_sector(struct fat_fs_info *fsi, uint8_t *buffer, uint32_t sector) {
	struct dev_module *devmod;
	size_t ret;
	int blk;
	int blkpersec = fsi->vi.bytepersec / fsi->bdev->block_size;
	devmod = fsi->bdev->dev_module;

	blk = sector * blkpersec;
	ret = bdev_read_blocks(devmod, buffer, blk, blkpersec);

	if (ret != fsi->vi.bytepersec)
		return DFS_ERRMISC;
	else
		return DFS_OK;
}

int fat_write_sector(struct fat_fs_info *fsi, uint8_t *buffer, uint32_t sector) {
	struct dev_module *devmod;
	size_t ret;
	int blk;
	int blkpersec = fsi->vi.bytepersec / fsi->bdev->block_size;
	devmod = fsi->bdev->dev_module;

	blk = sector * blkpersec;
	ret = bdev_write_blocks(devmod, buffer, blk, blkpersec);

	if (ret != fsi->vi.bytepersec)
		return DFS_ERRMISC;
	else
		return DFS_OK;
}

/**
 * @brief Fill dirent with dirinfo data
 *
 * @param di Directory
 * @param de Dirent to be filled
 *
 * @return Negative error code or zero if succeed
 */
static int fat_dirent_by_file(struct fat_file_info *fi, struct dirent *de) {
	struct fat_fs_info *fsi = fi->fsi;
	void *de_src;

	if (fat_read_sector(fsi, fat_sector_buff, fi->dirsector))
		return -1;

	de_src = &(((struct dirent*)fat_sector_buff)[fi->diroffset]);
	memcpy(de, de_src, sizeof(struct dirent));

	return 0;
}

/**
 * @brief Read directory info from drive and set currentcluster,
 * current sector
 *
 * @param di Directory to be updated. Di->fi.fsi should be set properly.
 *
 * @return Negative error code or zero if succeed
 */
static int fat_reset_dir(struct dirinfo *di) {
	struct dirent de;
	struct volinfo *vi;

	if (di->fi.dirsector == 0) {
		/* This is root dir */
		vi = &di->fi.fsi->vi;
		di->currentcluster = (vi->rootdir % vi->secperclus);
		di->currentsector = vi->rootdir % vi->secperclus;
	} else {
		fat_dirent_by_file(&di->fi, &de);
		di->currentcluster = (uint32_t) de.startclus_l_l |
		  ((uint32_t) de.startclus_l_h) << 8 |
		  ((uint32_t) de.startclus_h_l) << 16 |
		  ((uint32_t) de.startclus_h_h) << 24;
		di->currentsector = 0;
	}

	di->currententry = 0;

	return 0;
}

/**
 * @brief Set approptiate flags and i_data for given inode
 *
 * @param inode Inode to be filled
 * @param di FAT directory entry related to file
 *
 * @return Negative error code or zero if succeed
 */
static int fat_fill_inode(struct inode *inode, struct dirent *de, struct dirinfo *di) {
	struct fat_file_info *fi;
	struct dirinfo *new_di;
	struct volinfo *vi;
	struct super_block *sb;

	sb = inode->i_sb;
	vi = &((struct fat_fs_info*)sb->sb_data)->vi;
	if (de->attr & ATTR_DIRECTORY){
		if (NULL == (new_di = fat_dirinfo_alloc()))
			goto err_out;

		memset(new_di, 0, sizeof(struct dirinfo));
		new_di->p_scratch = fat_sector_buff;
		inode->flags |= S_IFDIR;

		new_di->currentcluster = (uint32_t) de->startclus_l_l |
		  ((uint32_t) de->startclus_l_h) << 8 |
		  ((uint32_t) de->startclus_h_l) << 16 |
		  ((uint32_t) de->startclus_h_h) << 24;

		fi = &new_di->fi;
	} else {
		if (NULL == (fi = fat_file_alloc()))
			goto err_out;

	}

	inode->i_data = fi;

	*fi = (struct fat_file_info) {
		.fsi = sb->sb_data,
		.volinfo = vi,
	};

	fi->dirsector = di->currentsector +
		di->currentcluster * vi->secperclus;
	fi->diroffset = di->currententry - 1;
	fi->cluster = (uint32_t) de->startclus_l_l |
	  ((uint32_t) de->startclus_l_h) << 8 |
	  ((uint32_t) de->startclus_h_l) << 16 |
	  ((uint32_t) de->startclus_h_h) << 24;
	fi->firstcluster = fi->cluster;
	fi->filelen = (uint32_t) de->filesize_0 |
			      ((uint32_t) de->filesize_1) << 8 |
			      ((uint32_t) de->filesize_2) << 16 |
			      ((uint32_t) de->filesize_3) << 24;

	inode->length    = fi->filelen;
	inode->start_pos = fi->firstcluster * fi->volinfo->secperclus * fi->volinfo->bytepersec;
	if (de->attr & ATTR_READ_ONLY)
		inode->flags |= S_IRALL;
	else
		inode->flags |= S_IRWXA;
	return 0;
err_out:
	return -1;
}

/**
* @brief Read related dir entries into dir buffer
*
* @param fsi Used to determine bdev and fat type (12/16/32)
* @param di  Pointer to dirinfo structure
*
* @return Negative error number
* @retval 0 Success
*/
static inline int read_dir_buf(struct fat_fs_info *fsi, struct dirinfo *di) {
	return fat_read_sector(fsi,
	                       di->p_scratch,
	                       fsi->vi.secperclus * di->currentcluster + di->currentsector);
}

/* @brief Figure out if node at specific path exists or not
 * @note  Assume dir is root
 * @note IMPORTANT: this functions should not be calls in the middle of iterate,
 * as it wipes dirinfo content
 *
 * @param name Full path to the extected node
 * @param dir  Not used now
 *
 * @return Pointer of inode or NULL if not found
 */
static struct inode *fat_ilookup(char const *name, struct dentry const *dir) {
	struct dirinfo *di;
	struct dirent de;
	struct super_block *sb;
	uint8_t tmp_ent;
	uint8_t tmp_sec;
	uint32_t tmp_clus;
	struct inode *node;
	char tmppath[PATH_MAX];
	char fat_name[12];
	int found = 0;
	struct fat_fs_info *fsi;

	assert(name);
	assert(dir->d_inode);
	assert(FILE_TYPE(dir->d_inode->flags, S_IFDIR));

	sb = dir->d_sb;
	di = dir->d_inode->i_data;

	assert(di);

	path_canonical_to_dir(fat_name, (char *) name);

	fsi = sb->sb_data;

	tmp_ent = di->currententry;
	tmp_sec = di->currentsector;
	tmp_clus = di->currentcluster;
	fat_reset_dir(di);

	if (read_dir_buf(fsi, di))
		goto err_out;

	while (!fat_get_next(fsi, di, &de)) {
		path_canonical_to_dir(tmppath, (char *) de.name);
		if (!memcmp(tmppath, fat_name, MSDOS_NAME)) {
			found = 1;
			break;
		}
	}

	if (!found)
		goto err_out;

	if (NULL == (node = dvfs_alloc_inode(sb)))
		goto err_out;

	if (fat_fill_inode(node, &de, di))
		goto err_out;

	goto succ_out;
err_out:
	node = NULL;
succ_out:
	di->currentcluster = tmp_clus;
	di->currententry = tmp_ent;
	di->currentsector = tmp_sec;
	return node;
}

/* @brief Create new file or directory
 * @param i_new Inode to be filled
 * @param i_dir Inode realted to the parent
 * @param mode  Used to figure out file type
 *
 * @return Negative error code
 */
static int fat_create(struct inode *i_new, struct inode *i_dir, int mode) {
	int res;
	uint32_t temp;
	struct fat_file_info *fi;
	struct fat_fs_info *fsi;
	struct dirinfo *di, *new_di;
	struct dirent de;
	uint32_t cluster;

	assert(i_new);
	assert(i_dir);

	/* TODO check file exists */
	fsi = i_dir->i_sb->sb_data;
	di = i_dir->i_data;

	fat_reset_dir(di);
	read_dir_buf(fsi, di);

	/* Find suitable dirent */
	while (DFS_OK == (res = fat_get_next(fsi, di, &de)));

	if (res != DFS_EOF)
		/* End of cluster, probably */
		/* TODO extend cluster chain */
		return -1;

	cluster = fat_get_free_fat_(fsi, fat_sector_buff);
	/* Be carefully, this functions explicitly erases de.attr */
	path_canonical_to_dir((char*) de.name, i_new->i_dentry->name);
	de.attr = S_ISDIR(mode) ? ATTR_DIRECTORY : 0;
	de.startclus_l_l = cluster & 0xff;
	de.startclus_l_h = (cluster & 0xff00) >> 8;
	de.startclus_h_l = (cluster & 0xff0000) >> 16;
	de.startclus_h_h = (cluster & 0xff000000) >> 24;

	if (FILE_TYPE(mode, S_IFDIR)) {
		if (!(new_di = fat_dirinfo_alloc()))
			return -ENOMEM;
		di->p_scratch = fat_sector_buff;
		fi = &new_di->fi;
	} else {
		if (!(fi = fat_file_alloc()))
			return -ENOMEM;
	}

	i_new->i_data = fi;

	*fi = (struct fat_file_info) {
		.fsi          = fsi,
		.volinfo      = &fsi->vi,
		.dirsector    = di->currentsector +
			di->currentcluster * fsi->vi.secperclus,
		.diroffset    = di->currententry,
		.cluster      = cluster,
		.firstcluster = cluster,
	};

	if (fat_read_sector(fsi, fat_sector_buff, fi->dirsector))
		return -1;

	memcpy(&(((struct dirent*) fat_sector_buff)[fi->diroffset]),
			&de, sizeof(struct dirent));
	if (fat_write_sector(fsi, fat_sector_buff, fi->dirsector))
		return -1;

	/* Mark newly allocated cluster as end of chain */
	switch(fsi->vi.filesystem) {
		case FAT12:		cluster = 0xfff;	break;
		case FAT16:		cluster = 0xffff;	break;
		case FAT32:		cluster = 0x0fffffff;	break;
		default:		return DFS_ERRMISC;
	}

	temp = 0;
	fat_set_fat_(fsi, fat_sector_buff, &temp, fi->cluster, cluster);

	return ENOERR;
}

static int fat_close(struct file *desc) {
	return 0;
}

static size_t fat_read(struct file *desc, void *buf, size_t size) {
	uint32_t res;
	struct fat_file_info *fi = desc->f_inode->i_data;
	fat_read_file(fi, fat_sector_buff, buf, &res, min(size, fi->filelen - desc->pos));
	return res;
}

static size_t fat_write(struct file *desc, void *buf, size_t size) {
	uint32_t res;
	struct fat_file_info *fi = desc->f_inode->i_data;
	fi->mode = O_RDWR; /* XXX */
	fat_write_file(fi, fat_sector_buff, buf, &res, size, &desc->f_inode->length);
	return res;
}

/* @brief Get next inode in directory
 * @param inode   Structure to be filled
 * @param parent  Inode of parent directory
 * @param dir_ctx Directory context
 *
 * @return Error code
 */
static int fat_iterate(struct inode *next, struct inode *parent, struct dir_ctx *ctx) {
	struct fat_fs_info *fsi;
	struct dirinfo *dirinfo;
	struct dirent de;
	char path[PATH_MAX];
	int res;

	if (!parent)
		strcpy(path, ROOT_DIR);

	assert(parent->i_sb);

	fsi = parent->i_sb->sb_data;
	dirinfo = parent->i_data;
	dirinfo->currententry = (int) ctx->fs_ctx;

	if (dirinfo->currententry == 0) {
		/* Need to get directory data from drive */
		fat_reset_dir(dirinfo);
	}

	read_dir_buf(fsi, dirinfo);

	while (((res = fat_get_next(fsi, dirinfo, &de)) ==  DFS_OK) || res == DFS_ALLOCNEW)
		if (de.name[0] == '\0')
			continue;
		else
			break;

	switch (res) {
	case DFS_OK:
		fat_fill_inode(next, &de, dirinfo);
		ctx->fs_ctx = (void*) ((int)dirinfo->currententry);
		return 0;
	case DFS_EOF:
		/* Fall through */
	default:
		return -1;
	}
}

static int fat_remove(struct inode *inode) {
	struct fat_file_info *fi;
	struct dirinfo *di;
	int res;

	if (FILE_TYPE(inode->flags, S_IFDIR)) {
		di = inode->i_data;
		res = fat_unlike_directory(&di->fi, NULL, (uint8_t*) fat_sector_buff);
	} else {
		fi = inode->i_data;
		res = fat_unlike_file(fi, NULL, (uint8_t*) fat_sector_buff);
	}
	return res;
}

static int fat_pathname(struct inode *inode, char *buf, int flags) {
	struct fat_file_info *fi;
	struct fat_fs_info *fsi;

	switch (flags) {
	case DVFS_NAME:
		fi = inode->i_data;
		fsi = fi->fsi;

		if (fat_read_sector(fsi, fat_sector_buff, fi->dirsector))
			return -1;
		strncpy(buf, (char*) ((struct dirent*) fat_sector_buff)[fi->diroffset].name, 11);
		buf[11] = '\0';
		return 0;
	default:
		/* NIY */
		return -ENOSYS;
	}
}

static int fat_truncate(struct inode *inode, size_t len) {
	/* This is a stub, but files should be extended automatically
	 * with the common part of the driver on write */
	return 0;
}

/* Declaration of operations */
struct inode_operations fat_iops = {
	.create   = fat_create,
	.lookup   = fat_ilookup,
	.remove   = fat_remove,
	.iterate  = fat_iterate,
	.pathname = fat_pathname,
	.truncate = fat_truncate,
};

struct file_operations fat_fops = {
	.close = fat_close,
	.write = fat_write,
	.read = fat_read,
};

static int fat_destroy_inode(struct inode *inode) {
	struct fat_file_info *fi;
	struct dirinfo *di;

	if (!inode->i_data)
		return 0;

	if (FILE_TYPE(inode->flags, S_IFDIR)) {
		di = inode->i_data;
		fat_dirinfo_free(di);
	} else {
		fi = inode->i_data;
		fat_file_free(fi);
	}

	return 0;
}

struct super_block_operations fat_sbops = {
	.open_idesc    = dvfs_file_open_idesc,
	.destroy_inode = fat_destroy_inode,
};


/**
 * @brief Fill given volinfo struct with info from first FS sector
 *
 * @note Actually, it's copy-paste from old fat driver, but with use
 * of devmodule, not raw block device.
 *
 * @param devmod Module related to block device
 * @param volinfo Structure to be filled
 *
 * @return Error code
 * @retval -1 Fail
 * @retval 0  Success
 */
static int fat_get_vi(struct dev_module *devmod, struct volinfo *volinfo) {
	struct lbr *lbr = (struct lbr *) fat_sector_buff;
	if (0 > bdev_read_block(devmod, lbr, 0))
		return -1;

	if (lbr->jump[0] != 0xeb || lbr->jump[2] != 0x90 ||
		lbr->sig_55 != 0x55 || lbr->sig_aa != 0xaa)
		return -1;

	volinfo->bytepersec = lbr->bpb.bytepersec_l + (lbr->bpb.bytepersec_h << 8);
	if (volinfo->bytepersec == 0)
		return -1;

	volinfo->startsector = 0;
	volinfo->secperclus = lbr->bpb.secperclus;
	volinfo->reservedsecs = (uint16_t) lbr->bpb.reserved_l |
		  (((uint16_t) lbr->bpb.reserved_h) << 8);

	volinfo->numsecs =  (uint16_t) lbr->bpb.sectors_s_l |
		  (((uint16_t) lbr->bpb.sectors_s_h) << 8);

	if (!volinfo->numsecs)
		volinfo->numsecs = (uint32_t) lbr->bpb.sectors_l_0 |
		  (((uint32_t) lbr->bpb.sectors_l_1) << 8) |
		  (((uint32_t) lbr->bpb.sectors_l_2) << 16) |
		  (((uint32_t) lbr->bpb.sectors_l_3) << 24);

	if (volinfo->numsecs == 0)
		return -1;

	/**
	 * If secperfat is 0, we must be in a FAT32 volume
	 */
	volinfo->secperfat =  (uint16_t) lbr->bpb.secperfat_l |
		  (((uint16_t) lbr->bpb.secperfat_h) << 8);

	if (!volinfo->secperfat) {
		volinfo->secperfat = (uint32_t) lbr->ebpb.ebpb32.fatsize_0 |
		  (((uint32_t) lbr->ebpb.ebpb32.fatsize_1) << 8) |
		  (((uint32_t) lbr->ebpb.ebpb32.fatsize_2) << 16) |
		  (((uint32_t) lbr->ebpb.ebpb32.fatsize_3) << 24);

		if (volinfo->secperfat == 0)
			return -1;

		memcpy(volinfo->label, lbr->ebpb.ebpb32.label, MSDOS_NAME);
		volinfo->label[11] = 0;
	} else {
		memcpy(volinfo->label, lbr->ebpb.ebpb.label, MSDOS_NAME);
		volinfo->label[11] = 0;
	}

	/* note: if rootentries is 0, we must be in a FAT32 volume. */
	volinfo->rootentries =  (uint16_t) lbr->bpb.rootentries_l |
		  (((uint16_t) lbr->bpb.rootentries_h) << 8);

	volinfo->fat1 = volinfo->reservedsecs;

	/**
	 * The calculation below is designed to round up the root directory size
	 * for FAT12/16 and to simply ignore the root directory for FAT32, since
	 * it's a normal, expandable file in that situation.
	 */

	if (volinfo->rootentries) {
		volinfo->rootdir = volinfo->reservedsecs + lbr->bpb.numfats * volinfo->secperfat;
		volinfo->dataarea = volinfo->rootdir
			+ (((volinfo->rootentries * 32) + (volinfo->bytepersec - 1))
			/ volinfo->bytepersec);
	} else {
		volinfo->dataarea = volinfo->fat1 + (volinfo->secperfat * 2);
		volinfo->rootdir = (uint32_t) lbr->ebpb.ebpb32.root_0 |
		  (((uint32_t) lbr->ebpb.ebpb32.root_1) << 8) |
		  (((uint32_t) lbr->ebpb.ebpb32.root_2) << 16) |
		  (((uint32_t) lbr->ebpb.ebpb32.root_3) << 24);
	}

	if (0 == volinfo->secperclus) {
		return -1;
	} else {
		volinfo->numclusters = (volinfo->numsecs - volinfo->dataarea) /
			volinfo->secperclus;
	}

	if (volinfo->numclusters < 4085)
		volinfo->filesystem = FAT12;
	else if (volinfo->numclusters < 65525)
		volinfo->filesystem = FAT16;
	else
		volinfo->filesystem = FAT32;

	return 0;
}

/* @brief Initializing fat super_block
 * @param sb  Structure to be initialized
 * @param dev Storage device
 *
 * @return Negative error code
 */
static int fat_fill_sb(struct super_block *sb, struct file *bdev_file) {
	struct fat_fs_info *fsi;
	struct block_dev *dev = bdev_file->f_inode->i_data;
	assert(sb);
	assert(dev);

	fsi = fat_fs_alloc();
	*fsi = (struct fat_fs_info) {
		.bdev = dev,
	};
	sb->sb_data = fsi;
	sb->sb_iops = &fat_iops;
	sb->sb_fops = &fat_fops;
	sb->sb_ops  = &fat_sbops;

	if (fat_get_vi(dev->dev_module, &fsi->vi))
		goto err_out;

	return 0;

err_out:
	fat_fs_free(fsi);
	return -1;
}

/**
* @brief Initialize dirinfo for root FAT directory
* @note  Should be called just after mounting FAT FS
*
* @param super_block Superblock of just
*
* @return Negative error code
* @retval 0 Success
*/
static int fat_mount_end(struct super_block *sb) {
	struct dirinfo *di;
	struct fat_fs_info *fsi;

	uint8_t tmp[] = { '\0' };
	assert(sb->bdev->block_size <= FAT_MAX_SECTOR_SIZE);

	if (NULL == (di = fat_dirinfo_alloc()))
		return -ENOMEM;

	di->p_scratch = fat_sector_buff;

	fsi = sb->sb_data;
	if (fat_open_dir(fsi, tmp, di))
		return -1;

	di->fi = (struct fat_file_info) {
		.fsi          = fsi,
		.volinfo      = &fsi->vi,
		.dirsector    = 0,
		.diroffset    = 0,
		.firstcluster = 0,
	};

	sb->root->d_inode->i_data = di;
	sb->root->d_inode->flags |= S_IFDIR;

	return 0;
}


/**
 * @brief Format given block device
 * @param dev Pointer to device
 * @note Should be block device
 *
 * @return Negative error code or 0 if succeed
 */
static int fat_format(void *dev, void *priv) {
	int fat_n = priv ? atoi((char*) priv) : 12;
	if (!fat_n)
		fat_n = 12;

	fat_create_partition(dev, fat_n);
	fat_root_dir_record(dev);

	return 0;
}

static struct dumb_fs_driver dfs_fat_driver = {
	.name      = "vfat",
	.fill_sb   = fat_fill_sb,
	.mount_end = fat_mount_end,
	.format    = fat_format,
};

ARRAY_SPREAD_DECLARE(struct dumb_fs_driver *, dumb_drv_tab);
ARRAY_SPREAD_ADD(dumb_drv_tab, &dfs_fat_driver);
