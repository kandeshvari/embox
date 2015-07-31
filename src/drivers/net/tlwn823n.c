/**
 * @file
 * @brief TP-LINK TL-WN823N v1 driver
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.1
 * @date 2015-06-16
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <drivers/usb/usb.h>
#include <drivers/usb/usb_driver.h>
#include <embox/unit.h>
#include <kernel/printk.h>
#include <kernel/sched/sched_lock.h>
#include <mem/sysmalloc.h>
#include <net/inetdevice.h>
#include <net/l2/ethernet.h>
#include <net/netdevice.h>
#include <basic_types.h>
#include <osdep_service.h>

static struct usb_device_id wn823n_id_table[] = {
	{ 0x0bda, 0x8178 },
	{ },
};

static struct usb_driver wn823n_driver;
static const struct net_driver wn823n_drv_ops;

EMBOX_UNIT_INIT(wn823n_init);

static int wn823n_init(void) {
	return usb_driver_register(&wn823n_driver);
}

struct wn823n_priv {
	struct usb_dev *usbdev;
	char *data;
	char *pdata;
	char *fw; /* Firmware */
	size_t fw_len;
};

#define REG_MACID	0x0610
#define	REALTEK_USB_VENQT_READ			0xC0
#define	REALTEK_USB_VENQT_WRITE			0x40
#define REALTEK_USB_VENQT_CMD_REQ		0x05
#define	REALTEK_USB_VENQT_CMD_IDX		0x00

static void wn823n_hnd(struct usb_request *req, void *arg) {
}

static int rtl_read_byte(struct wn823n_priv *priv, int addr, void *data) {
	usb_endp_control(priv->usbdev->endpoints[0],
			wn823n_hnd,
			NULL,
			REALTEK_USB_VENQT_READ,
			REALTEK_USB_VENQT_CMD_REQ,
			addr,
			REALTEK_USB_VENQT_CMD_IDX,
			1,
			data);
	return 0;
}

extern int rtw_drv_init(struct usb_interface *, const struct usb_device_id *);
extern int ksleep(useconds_t msec);
static void *wn823n_probe_hnd(void *arg) {
	static struct usb_device_id did;
	struct usb_dev *dev = (struct usb_dev *) arg;

	printk("Thread started\n");

	static struct usb_interface intf = {
		.altsetting = {
			{
				.desc = {
					/* This data could be obtained runtime */
					.b_num_endpoints      =   4,
					.b_length             =   9,
					.b_desc_type    =   4,
					.b_interface_number   =   0,
					.b_alternate_setting  =   0,
					.b_interface_class    = 255,
					.b_interface_subclass = 255,
					.b_interface_protocol = 255,
					.i_interface          =   0,
				},
			.endpoint = NULL,
			}
		},
	};

	ksleep(1000);

	intf.altsetting[0].endpoint = dev->endpoints;
	intf.altsetting[0].endpoint_desc = dev->getconf_data->endp_descs;

	intf.dev = arg;
	rtw_drv_init(&intf, &did);
	return 0;
}

static int wn823n_probe(struct usb_driver *drv, struct usb_dev *dev, void **data) {
	thread_create(THREAD_FLAG_KERNEL, wn823n_probe_hnd, dev);
	printk("Probe exit\n");
	return 0;
}

static void wn823n_disconnect(struct usb_dev *dev, void *data) {
	printk("wn823n disconnect\n");
}

static int wn823n_xmit(struct net_device *dev, struct sk_buff *skb) {
	printk("wn823n xmit\n");
	return 0;
}

static int wn823n_get_macaddr(struct net_device *dev, void *buff) {
	char *str = buff;
	struct wn823n_priv *priv;
	int i;
	assert(dev);
	assert(buff);

	printk("wn823n get macaddr\n");

	priv = netdev_priv(dev, struct wn823n_priv);

	for (i = 0; i < 6; i++)
		rtl_read_byte(priv, REG_MACID + i, str + i);

	return 0;
}

static struct usb_driver wn823n_driver = {
	.probe = wn823n_probe,
	.disconnect = wn823n_disconnect,
	.id_table = wn823n_id_table,
};

static const struct net_driver wn823n_drv_ops = {
	.xmit        = wn823n_xmit,
	.get_macaddr = wn823n_get_macaddr,
};
