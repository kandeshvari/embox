/**
 * @file iwconfig.c
 * @brief Configure wireless connections
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.0.1
 * @date 2015-06-26
 */

#include <stdio.h>
#include <drivers/usb/usb_driver.h>

/* realtek stuff */
#include <basic_types.h>
#include <osdep_service.h>

extern int rtw_drv_init(struct usb_interface *, const struct usb_device_id *);

int main(int argc, char **argv) {
	rtw_drv_init(NULL, NULL);
	return 0;
}
