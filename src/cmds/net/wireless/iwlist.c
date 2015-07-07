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

int print_usage(void) {
	printf("Usage example:\niwlist wlan0\n");
	return 0;
}

int main(int argc, char **argv) {
	/* Assert that interface is wireless */
	struct in_device *idev = inetdev_get_by_name(argv[1]);
	union iwreq_data wrqu;
	char buf[1024];

	if (argc < 2)
		return print_usage();

	if (rtw_wx_set_scan(idev->dev, NULL, NULL, NULL) < 0) {
		printf("Couldn't initialize %s for scanning. Are you sure iface is up?\n", argv[1]);
		return 0;
	}

	memset(buf, 0, sizeof(buf));

	wrqu.data = (struct iw_point) {
		.pointer = NULL,
		.flags = 0,
		.length = 0,
	};

	rtw_wx_get_scan(idev->dev, NULL, &wrqu, buf);

	printf("Scan result:\n %s\n", buf);

	return 0;
}
