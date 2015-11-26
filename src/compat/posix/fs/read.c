/**
 * @file
 *
 * @date 15.11.13
 * @author Anton Bondarev
 */

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>

#include <fs/index_descriptor.h>
#include <fs/idesc.h>
#include <kernel/task/resource/idesc_table.h>

ssize_t read(int fd, void *buf, size_t nbyte) {
	ssize_t ret;
	struct idesc *idesc;

	if (!idesc_index_valid(fd)
			|| (NULL == (idesc = index_descriptor_get(fd)))
			|| (!(idesc->idesc_amode & S_IROTH))) {
		return SET_ERRNO(EBADF);
	}

	assert(idesc->idesc_ops);
	assert(idesc->idesc_ops->read);
	ret = idesc->idesc_ops->read(idesc, buf, nbyte);
	if (ret < 0) {
		return SET_ERRNO(-ret);
	}

	return ret;
}
