#include <zephyr.h>
#include <devicetree.h>
#include <init.h>
#include <device.h>

#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>
#include <drivers/gpio.h>

#include <stdint.h>

#define LOG_LEVEL LOG_LEVEL_WRN
LOG_MODULE_REGISTER(main);

#define LED0_NODE            DT_ALIAS(led0)
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	             DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	             DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	             DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif

#define GET_REPORT_SIZE      64

#define HID_LI_USAGE2        0x0A /* Usage on 2 bytes */
#define HID_MI_FEATURE       0xB1
#define HID_GI_REPORT_COUNT2 0x96 /* Report count on 2 bytes */

static const struct device *hdev;


const uint8_t hid_report_desc[] = {
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),       /* USAGE_PAGE (Gen Desktop) */
	HID_LI_USAGE2, 0x00, 0xFF,                   /* USAGE (Vendor defined)   */
	HID_COLLECTION(HID_COLLECTION_APPLICATION),  /* COLLECTION (Application)*/
	HID_LOGICAL_MIN8(0),                         /*  LOGICAL_MINIMUM (0)    */
	HID_LOGICAL_MAX8(0xFF),                      /*  LOGICAL_MAXIMUM (255)  */
	HID_REPORT_SIZE(8),                          /*  REPORT_SIZE (8)        */
	HID_GI_REPORT_COUNT2, 0xFF, 0x00,            /*  REPORT_COUNT (255)     */
	HID_LI_USAGE2, 0x01, 0xFF,                   /*  USAGE (Vendor defined) */
	HID_MI_FEATURE, 0x02,                        /*  FEATURE (D,Var,A)      */
	HID_END_COLLECTION,                          /* END_COLLECTION          */
};

const uint16_t hid_report_desc_size = sizeof(hid_report_desc);

static uint8_t ret_report[GET_REPORT_SIZE]= {0};

bool led_is_on = true;

static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	switch (status) {
	case USB_DC_CONFIGURED:
		LOG_DBG("Usb is now configured");
		break;
	case USB_DC_SOF:
		break;
	default:
		LOG_DBG("status %u unhandled", status);
		break;
	}
}

int test_get_report(const struct device *dev, struct usb_setup_packet *setup,
		    int32_t *len, uint8_t **data)
{
	LOG_DBG("Get cb");
	*data = ret_report;
	*len = GET_REPORT_SIZE;

	led_is_on = !led_is_on;
	return 0;

}

int test_set_report(const struct device *dev, struct usb_setup_packet *setup,
		    int32_t *len, uint8_t **data)
{
	LOG_DBG("Set cb %d", *len);
	memcpy(ret_report, *data, *len);
	led_is_on = !led_is_on;
	return 0;
}

static const struct hid_ops ops = {
	.get_report = test_get_report,
	.set_report = test_set_report,
};

void main(void)
{
	const struct device *dev;
	int ret;

	if (usb_enable(status_cb) != 0) {
		LOG_ERR("Failed to enable USB");
	}

	dev = device_get_binding(LED0);
	if (dev == NULL) {
		return;
	}

	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		return;
	}

	while (1) {
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
		gpio_pin_set(dev, PIN, (int)led_is_on);
#endif
		k_sleep(K_MSEC(5));
	}
}

static int composite_pre_init(const struct device *dev)
{
	hdev = device_get_binding("HID_0");
	if (hdev == NULL) {
		LOG_ERR("Cannot get USB HID Device");
		return -ENODEV;
	}

	LOG_DBG("HID Device: dev %p", hdev);

	usb_hid_register_device(hdev, hid_report_desc, hid_report_desc_size,
				&ops);

	return usb_hid_init(hdev);
}

SYS_INIT(composite_pre_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
