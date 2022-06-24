# gpio_card_pcittl32io
A gpiochip compatible driver for the Quancom PCITTL32IO PCI GPIO card

## Overview

This repository contais the source code for a Linux PCI driver for the Quancom PCITTL32IO PCI GPIO Card. The driver should show be used as an example for how to write a PCI driver in Linux. It will support the gpiochip implementation, so you can use the gpiod tools like gpioset and gpioget to access the GPIO Pins of this card.

The documentation of this card can be found on the [Quancom website](http://quancom.de/qprod01/deu/pb/pci_io_card.htm).

## Video Tutorials

If you want to understand how Linux PCI Drivers work and how I incrementally developed this driver, you can watch my playlist over on [Youtube](https://www.youtube.com/watch?v=454KPcO95jY&list=PLCGpd0Do5-I3HkdOJ6SaFwNJgAcj8cDAh) or watch the video on [my Odysee channel](https://odysee.com/@Johannes4GNU_Linux:9).

## Support my work

If you want to support my work, you can buy me a coffe on [https://www.buymeacoffee.com/johannes4linux](https://www.buymeacoffee.com/johannes4linux).
