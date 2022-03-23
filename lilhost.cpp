extern "C" {
	#include <imports.h>
}

#include <pci/pci.h>
#include <timer/timer.h>
#include <utils/abort.h>

#include <memory/heap.h>
#include <memory/page_table_manager.h>

#include <lil_device.h>

extern "C" {
	void lil_pci_writeb(void* device, uint16_t offset, uint8_t val) {
		lil_pci_device* dev = (lil_pci_device*) device;
		pci::pci_writeb(dev->bus, dev->device, dev->function, offset, val);
	}

	void lil_pci_writew(void* device, uint16_t offset, uint16_t val) {
		lil_pci_device* dev = (lil_pci_device*) device;
		pci::pci_writew(dev->bus, dev->device, dev->function, offset, val);
	}

	void lil_pci_writed(void* device, uint16_t offset, uint32_t val) {
		lil_pci_device* dev = (lil_pci_device*) device;
		pci::pci_writed(dev->bus, dev->device, dev->function, offset, val);
	}

	uint8_t lil_pci_readb(void* device, uint16_t offset) {
		lil_pci_device* dev = (lil_pci_device*) device;
		return pci::pci_readb(dev->bus, dev->device, dev->function, offset);
	}

	uint16_t lil_pci_readw(void* device, uint16_t offset) {
		lil_pci_device* dev = (lil_pci_device*) device;
		return pci::pci_readw(dev->bus, dev->device, dev->function, offset);
	}

	uint32_t lil_pci_readd(void* device, uint16_t offset) {
		lil_pci_device* dev = (lil_pci_device*) device;
		return pci::pci_readd(dev->bus, dev->device, dev->function, offset);
	}

	void lil_sleep(uint64_t ms) {
		timer::global_timer->sleep(ms);
	}

	void lil_panic(const char* msg) {
		abortf("%s", msg);
	}

	void* lil_malloc(size_t s) {
		return memory::malloc(s);
	}

	__attribute__((naked)) void free(void* p) {
		asm volatile(
			".global _ZN6memory4freeEPv;"
			"jmp _ZN6memory4freeEPv;"
		);
	}

	// ^ why the f not lil_free?

	void* lil_map(size_t addr, size_t bytes) {
		// page align the address
		size_t aligned_addr = addr & ~0xFFF;

		// add the bits we start earlier to page align the address
		bytes += aligned_addr - addr;

		// make sure its at least 4k
		bytes = bytes < 0x1000 ? 0x1000 : bytes;

		memory::global_page_table_manager.map_range((void*) aligned_addr, (void*) aligned_addr, bytes);

		return (void*) addr;
	}
}