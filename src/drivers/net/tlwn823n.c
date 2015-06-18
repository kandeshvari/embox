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

#include <drivers/usb/usb_driver.h>
#include <embox/unit.h>
#include <kernel/printk.h>
#include <mem/sysmalloc.h>
#include <net/inetdevice.h>
#include <net/l2/ethernet.h>
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

struct wn823n_priv {
	struct usb_dev *usbdev;
	char *data;
	char *pdata;
	char *fw; /* Firmware */
	size_t fw_len;
};

#define REG_MACID	0x0610
static int rtl_read_byte(int addr) {
	/* Stub */
	return 0xff;
}

static int rtl_write_byte(int addr, char *data) {
	/* Stub */
	return 0;
}

#define FW_NAME       "firmware/rtl8192cufw.bin"
#define FW_MAX_LEN    0x4000
#define FW_START_ADDR 0x1000

static int wn823n_load_firmware(struct wn823n_priv *priv) {
	FILE *fs_fw;
	int i;
	assert(priv);

	if (NULL == (fs_fw = fopen(FW_NAME, "r")))
		return -EINVAL;

	if ((priv->fw_len = fread(priv->fw, FW_MAX_LEN, 1, fs_fw)) < 0) {
		return priv->fw_len;
	}

	for (i = 0; i < priv->fw_len; i++) {
		if (rtl_write_byte(FW_START_ADDR + i, priv->fw + i))
			return -1;
	}

	return 0;
}

static int wn823n_probe(struct usb_driver *drv, struct usb_dev *dev, void **data) {
	struct net_device *nic;
	struct wn823n_priv *nic_priv;
	int errcode;
	assert(drv);
	assert(dev);
	assert(data);

	printk("wn823n probe\n");
	/* TODO etherdev sets ethXX as devname, while wlanXX should be used */
	nic = (struct net_device *) etherdev_alloc(sizeof *nic_priv);
	if (!nic)
		return -ENOMEM;

	nic->drv_ops = &wn823n_drv_ops;
	nic_priv = netdev_priv(nic, struct wn823n_priv);
	nic_priv->fw = sysmalloc(FW_MAX_LEN);
	wn823n_load_firmware(nic_priv);

	nic_priv->data = nic_priv->pdata = sysmalloc(ETH_FRAME_LEN);
	if (!nic_priv->data) {
		etherdev_free(nic);
		return -ENOMEM;
	}

	*nic_priv = (struct wn823n_priv) {
		.usbdev = dev,
	};

	if ((errcode = inetdev_register_dev(nic)) < 0) {
		etherdev_free(nic);
		return errcode;
	}
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
	int i;
	assert(dev);
	assert(buff);

	printk("wn823n get macaddr\n");
	for (i = 0; i < 6; i++)
		str[i] = rtl_read_byte(REG_MACID + i);

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
