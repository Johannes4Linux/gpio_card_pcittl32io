#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/gpio.h>

#define PCITTL32IO_GPIO_STATE 0xFC
#define PCITTL32IO_DIRECTION  0xF8
#define PCITTL32IO_OFFSET_IRQ  0xF9


#define PCITTL32IO_VENDOR_ID 0x8008
#define PCITTL32IO_DEVICE_ID 0x3301

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("Driver for the Quancom PCITTL32IO gpio card");

static struct pci_device_id pcittl32io_ids[] = {
	{ PCI_DEVICE(PCITTL32IO_VENDOR_ID, PCITTL32IO_DEVICE_ID) },
	{ }
};
MODULE_DEVICE_TABLE(pci, pcittl32io_ids);

struct pcittl32io_gpiochip {
	void __iomem *ptr_bar0;
	struct gpio_chip chip;
};

static void pcittl32io_set_multiple(struct gpio_chip *chip, unsigned long *mask, unsigned long *bits) {
	u32 val;
	struct pcittl32io_gpiochip *gpio = gpiochip_get_data(chip);

	val = ioread32(gpio->ptr_bar0 + PCITTL32IO_GPIO_STATE);
	val = (val & *mask) | *bits;
	iowrite32(val, gpio->ptr_bar0 + PCITTL32IO_GPIO_STATE);
}

static void pcittl32io_set(struct gpio_chip *chip, unsigned gpio_nr, int value) {
	unsigned long mask = 0, bits = 0;
	mask = ~(1 << gpio_nr);
	bits = (value << gpio_nr);
	pcittl32io_set_multiple(chip, &mask, &bits);
}

static int pcittl32io_get_multiple(struct gpio_chip *chip, unsigned long *mask, unsigned long *bits) {
	u32 val;
	struct pcittl32io_gpiochip *gpio = gpiochip_get_data(chip);

	val = ioread32(gpio->ptr_bar0 + PCITTL32IO_GPIO_STATE);
	*bits = (val & *mask);
	return 0;
}
	
static int pcittl32io_get(struct gpio_chip *chip, unsigned gpio_nr) {
	unsigned long mask = 0, bits = 0;

	mask = (1< gpio_nr);
	pcittl32io_get_multiple(chip, &mask, &bits);
	return (bits > 0) ? 1 : 0;
}

static int pcittl32io_get_direction(struct gpio_chip *chip, unsigned gpio_nr) {
	u8 val;
	struct pcittl32io_gpiochip *gpio = gpiochip_get_data(chip);

	val = ioread8(gpio->ptr_bar0 + PCITTL32IO_DIRECTION);
	val = val & (1<<(gpio_nr/8));
	return (val > 0) ? GPIO_LINE_DIRECTION_OUT : GPIO_LINE_DIRECTION_IN;
}

static int pcittl32io_set_direction_input(struct gpio_chip *chip, unsigned gpio_nr) {
	u8 val;
	struct pcittl32io_gpiochip *gpio = gpiochip_get_data(chip);

	val = ioread8(gpio->ptr_bar0 + PCITTL32IO_DIRECTION);
	val &= ~(1 << (gpio_nr / 8));
	iowrite8(val, gpio->ptr_bar0 + PCITTL32IO_DIRECTION);
	return 0;
}

static int pcittl32io_set_direction_output(struct gpio_chip *chip, unsigned gpio_nr, int value) {
	u8 val;
	struct pcittl32io_gpiochip *gpio = gpiochip_get_data(chip);

	val = ioread8(gpio->ptr_bar0 + PCITTL32IO_DIRECTION);
	val |= (1 << (gpio_nr / 8));
	iowrite8(val, gpio->ptr_bar0 + PCITTL32IO_DIRECTION);
	pcittl32io_set(chip, gpio_nr, value);
	return 0;
}

static const struct gpio_chip template_chip = {
	.label = "pcittl32io",
	.owner = THIS_MODULE,
	/* Function to control gpios */
	.get_direction = pcittl32io_get_direction,
	.direction_input = pcittl32io_set_direction_input,
	.direction_output = pcittl32io_set_direction_output,
	.set = pcittl32io_set,
	.set_multiple = pcittl32io_set_multiple,
	.get = pcittl32io_get,
	.get_multiple = pcittl32io_get_multiple,
	.base = -1,
	.ngpio = 32,
	.can_sleep = true,
};


/**
 * @brief Function is called, when a PCI device is registered
 *
 * @param dev   pointer to the PCI device
 * @param id    pointer to the corresponding id table's entry
 *
 * @return      0 on success
 *              negative error code on failure
 */
static int pcittl32io_probe(struct pci_dev *dev, const struct pci_device_id *id) {
	struct pcittl32io_gpiochip *my_data;
	int status;

	printk("pcittl32io - Now I am in the probe function.\n");

	status = pci_resource_len(dev, 0);
	printk("pcittl32io - BAR0 is %d bytes in size\n", status);
	if(status != 256) {
		printk("pcittl32io - Wrong size of BAR0!\n");
		return -1;
	}

	printk("pcittl32io - BAR0 is mapped to 0x%llx\n", pci_resource_start(dev, 0));

	status = pcim_enable_device(dev);
	if(status < 0) {
		printk("pcittl32io - Could not enable device\n");
		return status;
	}

	status = pcim_iomap_regions(dev, BIT(0), KBUILD_MODNAME);
	if(status < 0) {
		printk("pcittl32io - BAR0 is already in use!\n");
		return status;
	}

	my_data = devm_kzalloc(&dev->dev, sizeof(struct pcittl32io_gpiochip), GFP_KERNEL);
	if(my_data == NULL) {
		printk("pcittl32io - Error! Out of memory\n");
		return -ENOMEM;
	}

	my_data->ptr_bar0 = pcim_iomap_table(dev)[0];
	if(my_data->ptr_bar0 == NULL) {
		printk("pcittl32io - BAR0 pointer is invalid\n");
		return -1;
	}

	my_data->chip = template_chip;
	if(gpiochip_add_data(&my_data->chip, my_data) < 0) {
		printk("pcittl32io - Error! Can't add gpiochip\n");
		return -1;
	}

	pci_set_drvdata(dev, my_data);

	return 0;
}

/**
 * @brief Function is called, when a PCI device is unregistered
 *
 * @param dev   pointer to the PCI device
 */
static void pcittl32io_remove(struct pci_dev *dev) {
	struct pcittl32io_gpiochip * my_data = pci_get_drvdata(dev);
	printk("pcittl32io - Now I am in the remove function.\n");
	if(my_data) 
		gpiochip_remove(&my_data->chip);
}

/* PCI driver struct */
static struct pci_driver pcittl32io_driver = {
	.name = "pcittl32io",
	.id_table = pcittl32io_ids,
	.probe = pcittl32io_probe,
	.remove = pcittl32io_remove,
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("pcittl32io - Registering the PCI device\n");
	return pci_register_driver(&pcittl32io_driver);
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("pcittl32io - Unregistering the PCI device\n");
	pci_unregister_driver(&pcittl32io_driver);
}

module_init(my_init);
module_exit(my_exit);


