#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>

#define PCITTL32IO_GPIO_STATE 0xFC
#define PCITTL32IO_DIRECTION  0xF8


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
	void __iomem *ptr_bar0;
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

	ptr_bar0 = pcim_iomap_table(dev)[0];
	if(ptr_bar0 == NULL) {
		printk("pcittl32io - BAR0 pointer is invalid\n");
		return -1;
	}

	printk("pcittl32io - GPIO State DWord 0x%x\n", ioread32(ptr_bar0 + PCITTL32IO_GPIO_STATE));
	iowrite8(0x1, ptr_bar0 + PCITTL32IO_DIRECTION);

	return 0;
}

/**
 * @brief Function is called, when a PCI device is unregistered
 *
 * @param dev   pointer to the PCI device
 */
static void pcittl32io_remove(struct pci_dev *dev) {
	printk("pcittl32io - Now I am in the remove function.\n");
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


