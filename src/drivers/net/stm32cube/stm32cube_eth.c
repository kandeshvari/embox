/**
 * @file
 * @brief
 *
 * @author  Anton Kozlov
 * @date    07.08.2014
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <util/math.h>

#include <kernel/irq.h>
#include <hal/reg.h>

#include <net/netdevice.h>
#include <net/inetdevice.h>
#include <net/l0/net_entry.h>
#include <net/l2/ethernet.h>
#include <net/l3/arp.h>

#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_eth.h>

#include <util/log.h>

#include <embox/unit.h>

EMBOX_UNIT_INIT(stm32eth_init);

#define STM32ETH_IRQ (ETH_IRQn + 16)

#define PHY_ADDRESS       0x01 /* Relative to STM324xG-EVAL Board */

static ETH_HandleTypeDef stm32_eth_handler;


void HAL_ETH_MspInit(ETH_HandleTypeDef *heth) {
	/*(##) Enable the Ethernet interface clock using
	 (+++) __HAL_RCC_ETHMAC_CLK_ENABLE();
	 (+++) __HAL_RCC_ETHMACTX_CLK_ENABLE();
	 (+++) __HAL_RCC_ETHMACRX_CLK_ENABLE();

	 (##) Initialize the related GPIO clocks
	 (##) Configure Ethernet pin-out
	 (##) Configure Ethernet NVIC interrupt (IT mode)
	 */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable ETHERNET clock
	__HAL_RCC_ETHMAC_CLK_ENABLE();
	__HAL_RCC_ETHMACTX_CLK_ENABLE();
	__HAL_RCC_ETHMACRX_CLK_ENABLE();
	*/
	__HAL_RCC_ETH_CLK_ENABLE();

	/* Enable GPIOs clocks */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	/* Ethernet pins configuration ************************************************/
	/*
	 ETH_MDIO --------------> PA2
	 ETH_MDC ---------------> PC1

	 ETH_RMII_REF_CLK-------> PA1

	 ETH_RMII_CRS_DV -------> PA7
	 ETH_MII_RX_ER   -------> PB10
	 ETH_RMII_RXD0   -------> PC4
	 ETH_RMII_RXD1   -------> PC5
	 ETH_RMII_TX_EN  -------> PB11
	 ETH_RMII_TXD0   -------> PB12
	 ETH_RMII_TXD1   -------> PB13

	 ETH_RST_PIN     -------> PE2
	 */

	/* Configure PA1,PA2 and PA7 */
	GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PB10,PB11,PB12 and PB13 */
	GPIO_InitStructure.Pin =
			GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12	| GPIO_PIN_13;
	/* GPIO_InitStructure.Alternate = GPIO_AF11_ETH; */
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure PC1, PC4 and PC5 */
	GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
	/* GPIO_InitStructure.Alternate = GPIO_AF11_ETH; */
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	if (heth->Init.MediaInterface == ETH_MEDIA_INTERFACE_MII) {
		/* Output HSE clock (25MHz) on MCO pin (PA8) to clock the PHY */
		HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
	}

	/* Configure the PHY RST  pin */
	GPIO_InitStructure.Pin = GPIO_PIN_2;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_Delay(1);
}

static ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB]__attribute__ ((aligned (4)));
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__ ((aligned (4)));

static ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB]__attribute__ ((aligned (4)));
static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__ ((aligned (4)));

static void low_level_init(unsigned char mac[6]) {
	//uint32_t regvalue;
	int err;

	memset(&stm32_eth_handler, 0, sizeof(stm32_eth_handler));

	stm32_eth_handler.Instance = (ETH_TypeDef *) ETH_BASE;
	/* Fill ETH_InitStructure parametrs */
	stm32_eth_handler.Init.MACAddr = mac;
	stm32_eth_handler.Init.AutoNegotiation = ETH_AUTONEGOTIATION_DISABLE;
	//stm32_eth_handler.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
	stm32_eth_handler.Init.Speed = ETH_SPEED_100M;
	stm32_eth_handler.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
	stm32_eth_handler.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
	stm32_eth_handler.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;//ETH_CHECKSUM_BY_HARDWARE;
	stm32_eth_handler.Init.PhyAddress = PHY_ADDRESS;
	stm32_eth_handler.Init.RxMode = ETH_RXINTERRUPT_MODE;

	if (HAL_OK != (err = HAL_ETH_Init(&stm32_eth_handler))) {
		log_error("HAL_ETH_Init err %d\n", err);
	}
	if (stm32_eth_handler.State == HAL_ETH_STATE_READY) {
		log_error("STATE_READY sp %d duplex %d\n", stm32_eth_handler.Init.Speed, stm32_eth_handler.Init.DuplexMode);
	}

	/*(#)Initialize Ethernet DMA Descriptors in chain mode and point to allocated buffers:*/
	HAL_ETH_DMATxDescListInit(&stm32_eth_handler, DMATxDscrTab, &Tx_Buff[0][0],
			ETH_TXBUFNB); /*for Transmission process*/
	if (HAL_OK != (err = HAL_ETH_DMARxDescListInit(&stm32_eth_handler, DMARxDscrTab, &Rx_Buff[0][0],
			ETH_RXBUFNB))) { /*for Reception process*/
		log_error("HAL_ETH_DMARxDescListInit %d\n", err);
	}

	/* (#)Enable MAC and DMA transmission and reception: */
	HAL_ETH_Start(&stm32_eth_handler);
#if 0
	/**** Configure PHY to generate an interrupt when Eth Link state changes ****/
	/* Read Register Configuration */
	HAL_ETH_ReadPHYRegister(&stm32_eth_handler, PHY_MICR, &regvalue);

	regvalue |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);

	/* Enable Interrupts */
	HAL_ETH_WritePHYRegister(&stm32_eth_handler, PHY_MICR, regvalue);

	/* Read Register Configuration */
	HAL_ETH_ReadPHYRegister(&stm32_eth_handler, PHY_MISR, &regvalue);

	regvalue |= PHY_MISR_LINK_INT_EN;

	/* Enable Interrupt on change of link status */
	HAL_ETH_WritePHYRegister(&stm32_eth_handler, PHY_MISR, regvalue);
#endif
}


#define DEBUG 0
#if DEBUG
#include <kernel/printk.h>
/* Debugging routines */
static inline void show_packet(uint8_t *raw, int size, char *title) {
	int i;

	irq_lock();
	printk("\nPACKET(%d) %s:", size, title);
	for (i = 0; i < size; i++) {
		if (!(i % 16)) {
			printk("\n");
		}
		printk(" %02hhX", *(raw + i));
	}
	printk("\n.\n");
	irq_unlock();
}
#else
static inline void show_packet(uint8_t *raw, int size, char *title) {
}
#endif

static struct sk_buff *low_level_input(void) {
	struct sk_buff *skb;
	int len;
	uint8_t *buffer;
	uint32_t i=0;
	__IO ETH_DMADescTypeDef *dmarxdesc;

	skb = NULL;

	/* get received frame */
	if (HAL_ETH_GetReceivedFrame_IT(&stm32_eth_handler) != HAL_OK)
		return NULL;

	/* Obtain the size of the packet and put it into the "len" variable. */
	len = stm32_eth_handler.RxFrameInfos.length;
	buffer = (uint8_t *) stm32_eth_handler.RxFrameInfos.buffer;

	/* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
	skb = skb_alloc(len);

	/* copy received frame to pbuf chain */
	if (skb != NULL) {
		memcpy(skb->mac.raw, buffer, len);
	}

	  /* Release descriptors to DMA */
	  dmarxdesc = stm32_eth_handler.RxFrameInfos.FSRxDesc;

	  /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
	  for (i=0; i< stm32_eth_handler.RxFrameInfos.SegCount; i++)
	  {
	    dmarxdesc->Status |= ETH_DMARXDESC_OWN;
	    dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
	  }

	  /* Clear Segment_Count */
	  stm32_eth_handler.RxFrameInfos.SegCount =0;


	  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
	  if ((stm32_eth_handler.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
	  {
	    /* Clear RBUS ETHERNET DMA flag */
		  stm32_eth_handler.Instance->DMASR = ETH_DMASR_RBUS;
	    /* Resume DMA reception */
		  stm32_eth_handler.Instance->DMARPDR = 0;
	  }
	return skb;
}


static int stm32eth_xmit(struct net_device *dev, struct sk_buff *skb);
static int stm32eth_open(struct net_device *dev);
static int stm32eth_set_mac(struct net_device *dev, const void *addr);
static const struct net_driver stm32eth_ops = {
		.xmit = stm32eth_xmit,
		.start = stm32eth_open,
		.set_macaddr = stm32eth_set_mac,
};

static int stm32eth_open(struct net_device *dev) {
	low_level_init(dev->dev_addr);
	return 0;
}

static int stm32eth_set_mac(struct net_device *dev, const void *addr) {
	memcpy(dev->dev_addr, addr, ETH_ALEN);
	return ENOERR;
}

static int stm32eth_xmit(struct net_device *dev, struct sk_buff *skb) {
	__IO ETH_DMADescTypeDef *dma_tx_desc;

	dma_tx_desc = stm32_eth_handler.TxDesc;
	memcpy((void *)dma_tx_desc->Buffer1Addr, skb->mac.raw, skb->len);

	/* Prepare transmit descriptors to give to DMA */
	HAL_ETH_TransmitFrame(&stm32_eth_handler, skb->len);

	skb_free(skb);

	return 0;
}

static irq_return_t stm32eth_interrupt(unsigned int irq_num, void *dev_id) {
	struct net_device *nic_p = dev_id;
	struct sk_buff *skb;
	ETH_HandleTypeDef *heth = &stm32_eth_handler;

	if (!nic_p) {
		return IRQ_NONE;
	}

	/* Frame received */
	if (__HAL_ETH_DMA_GET_FLAG(heth, ETH_DMA_FLAG_R)) {
		/* Receive complete callback */
		while (NULL != (skb = low_level_input())) {
			skb->dev = nic_p;

			show_packet(skb->mac.raw, skb->len, "rx");
			netif_rx(skb);
		}
		/* Clear the Eth DMA Rx IT pending bits */
		__HAL_ETH_DMA_CLEAR_IT(heth, ETH_DMA_IT_R);
	}
#if 0
	if (__HAL_ETH_DMA_GET_FLAG(heth, ETH_DMA_FLAG_T)) {
		__HAL_ETH_DMA_CLEAR_IT(heth, ETH_DMA_FLAG_T);
	}
#endif
	__HAL_ETH_DMA_CLEAR_IT(heth, ETH_DMA_IT_NIS);
	return IRQ_HANDLED;
}

static struct net_device *stm32eth_netdev;
static int stm32eth_init(void) {
	int res;
	struct net_device *nic;

	nic = (struct net_device *) etherdev_alloc(0);
	if (nic == NULL) {
		return -ENOMEM;
	}

	nic->drv_ops = &stm32eth_ops;
	nic->irq = STM32ETH_IRQ;
	nic->base_addr = ETH_BASE;
	/*nic_priv = netdev_priv(nic, struct stm32eth_priv);*/

	stm32eth_netdev = nic;

	res = irq_attach(nic->irq, stm32eth_interrupt, 0, stm32eth_netdev, "");
	if (res < 0) {
		return res;
	}

	return inetdev_register_dev(nic);
}

/* STM32ETH_IRQ defined through ETH_IRQn, which as enum and
 * can't expand into int, as STATIC_IRQ_ATTACH require
 */
static_assert(77 == STM32ETH_IRQ);
STATIC_IRQ_ATTACH(77, stm32eth_interrupt, stm32eth_netdev);
