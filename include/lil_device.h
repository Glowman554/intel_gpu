#pragma once

#include <pci/pci.h>

struct lil_pci_device {
	pci::pci_header_0_t* header;
	uint16_t bus;
	uint16_t device;
	uint16_t function;
};