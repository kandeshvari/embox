/**
 * @file iwlist.c
 * @brief List wireless networks
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.0.1
 * @date 2015-07-06
 */

#include <stdio.h>

#include <net/inetdevice.h>

#include <basic_types.h>
#include <osdep_service.h>

extern int rtw_wx_set_scan(struct net_device *dev, struct iw_request_info *a,
		union iwreq_data *wrqu, char *extra);
extern int rtw_wx_get_scan(struct net_device *dev, struct iw_request_info *a,
			union iwreq_data *wrqu, char *extra);

int main(int argc, char **argv) {
	/* Assert that interface is wireless */
	struct in_device *idev = inetdev_get_by_name(argv[1]);
	union iwreq_data wrqu;
	char buf[1024];

	if (rtw_wx_set_scan(idev->dev, NULL, NULL, NULL) < 0) {
		printf("Couldn't initialize device for scanning.\n");
		return 0;
	}

	wrqu.data = (struct iw_point) {
		.pointer = NULL,
		.flags = 0,
		.length = 0,
	};

	rtw_wx_get_scan(idev->dev, NULL, &wrqu, buf);

	return 0;
}
