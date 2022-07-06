#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>

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
	u16 vid, did;
	u8 capability_ptr;
	u32 bar0, saved_bar0;

	printk("pcittl32io - Now I am in the probe function.\n");

	/* Let's read the PCIe VID and DID */
	if(0 != pci_read_config_word(dev, 0x0, &vid)) {
		printk("pcittl32io - Error reading from config space\n");
		return -1;
	}
	printk("pcittl32io - VID; 0x%x\n", vid);
	if(0 != pci_read_config_word(dev, 0x2, &did)) {
		printk("pcittl32io - Error reading from config space\n");
		return -1;
	}
	printk("pcittl32io - DID; 0x%x\n", did);

	/* Read the pci capability pointer */
	printk("pcittl32io - VID; 0x%x\n", vid);
	if(0 != pci_read_config_byte(dev, 0x34, &capability_ptr)) {
		printk("pcittl32io - Error reading from config space\n");
		return -1;
	}
	if(capability_ptr) 
		printk("pcittl32io - PCI card has capabilities!\n");
	else
		printk("pcittl32io - PCI card doesn't have capabilities!\n");


	if(0 != pci_read_config_dword(dev, 0x10, &bar0)) {
		printk("pcittl32io - Error reading from config space\n");
		return -1;
	}

	saved_bar0 = bar0;

	if(0 != pci_write_config_dword(dev, 0x10, 0xffffffff)) {
		printk("pcittl32io - Error writing to config space\n");
		return -1;
	}

	if(0 != pci_read_config_dword(dev, 0x10, &bar0)) {
		printk("pcittl32io - Error reading from config space\n");
		return -1;
	}

	if((bar0 & 0x3) == 1) 
		printk("pcittl32io - BAR0 is IO space\n");
	else
		printk("pcittl32io - BAR0 is memory space\n");

	bar0 &= 0xFFFFFFFD;
	bar0 = ~bar0;
	bar0++;

	printk("pcittl32io - BAR0 is %d bytes big\n", bar0);


	if(0 != pci_write_config_dword(dev, 0x10, saved_bar0)) {
		printk("pcittl32io - Error writing to config space\n");
		return -1;
	}


	
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


