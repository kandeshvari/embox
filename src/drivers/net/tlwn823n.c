/**
 * @file
 * @brief TP-LINK TL-WN823N v1 driver
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.1
 * @date 2015-06-16
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <drivers/usb/usb_driver.h>
#include <embox/unit.h>
#include <kernel/printk.h>
#include <net/netdevice.h>

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

static int wn823n_probe(struct usb_driver *drv, struct usb_dev *dev,
		void **data) {
	printk("wn823n probe\n");
	return 0;
}

static void wn823n_disconnect(struct usb_dev *dev, void *data) {
	printk("wn823n disconnect\n");
}

static int wn823n_xmit(struct net_device *dev, struct sk_buff *skb) {
	printk("wn823n xmit\n");
	return 0;
}

static struct usb_driver wn823n_driver = {
	.probe = wn823n_probe,
	.disconnect = wn823n_disconnect,
	.id_table = wn823n_id_table,
};

static const struct net_driver wn823n_drv_ops = {
	.xmit = wn823n_xmit,
};
