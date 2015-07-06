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

/* TP-LINK TL-WN823N usb info */
#define RTL_VEND_ID 0x0bda
#define RTL_PROD_ID 0x8178

int main(int argc, char **argv) {
	struct usb_interface intf;
	struct usb_device_id did;
	struct usb_dev *usb_dev = NULL;

	while ((usb_dev = usb_dev_iterate(usb_dev))) {
		if (usb_dev->dev_desc.id_vendor == RTL_VEND_ID &&
				usb_dev->dev_desc.id_product == RTL_PROD_ID)
			break;
	}

	if (!usb_dev) {
		printf("WN823N is not registered usb device. Abort.\n");
		return 0;
	}

	memset(&intf, 0, sizeof(intf));
	memset(&did, 0, sizeof(did));

	intf.dev = usb_dev;

	rtw_drv_init(&intf, &did);
	return 0;
}
