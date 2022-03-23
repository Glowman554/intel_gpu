#include <elf/kernel_module.h>

#include <utils/log.h>

#include <renderer/font_renderer.h>
#include <renderer/renderer.h>
#include <memory/page_frame_allocator.h>

#include <pci/pci.h>

#include <lil_device.h>

extern "C" {
	#include <intel.h>
}

uintptr_t align_down(uintptr_t n, uintptr_t a) {
	return (n & ~(a - 1));
}

uintptr_t align_up(uintptr_t n, uintptr_t a) {
	return align_down(n + a - 1, a);
}

struct gpu_mode_t {
	int width;
	int height;
	uint64_t pitch;
	int bpp;
};

void set_mode(LilGpu* ctx, LilConnector* connector, LilConnectorInfo* connector_info, int gpu_mode_idx, gpu_mode_t* mode) {
	LilCrtc* crtc = connector->crtc;
	LilPlane* plane = &crtc->planes[0];

	crtc->current_mode = connector_info->modes[gpu_mode_idx];
	plane->surface_address = 0;
	plane->enabled = true;

	crtc->shutdown(ctx, crtc);
	ctx->vmem_clear(ctx);

	void* gtt_dummy = memory::global_allocator.request_page();
	void* lfb = memory::global_allocator.request_pages((mode->pitch * mode->height) / 0x1000 + 1);

	crtc->commit_modeset(ctx, crtc);
	plane->update_surface(ctx, plane, 0, mode->pitch);

	renderer::global_font_renderer->cursor_position = {0, 0};

	renderer::default_framebuffer = {
		.base_address = (void*)lfb,
		.buffer_size = mode->pitch * mode->height,
		.width = (uint32_t) mode->width,
		.height = (uint32_t) mode->height,
	};
}

void intel_gpu(pci::pci_header_0_t* header, uint16_t bus, uint16_t device, uint16_t function) {
	debugf("Found Intel GPU %x:%x\n", header->header.vendor_id, header->header.device_id);

	pci::become_bus_master(bus, device, function);
	pci::enable_mmio(bus, device, function);

	LilGpu* context = new LilGpu;
	LilConnector* connector = nullptr;
	LilConnectorInfo* connector_info = new LilConnectorInfo;
	lil_pci_device* device_info = new lil_pci_device;
	*device_info = {
		.header = header,
		.bus = bus,
		.device = device,
		.function = function,
	};

	gpu_mode_t modes[4];

	lil_init_gpu(context, device_info);

	for (int i = 0; i < context->num_connectors; i++) {
		LilConnector* _connector = &context->connectors[i];

		if (!connector->is_connected(context, _connector)) {
			continue;
		}

		*connector_info = _connector->get_connector_info(context, _connector);

		for (int j = 0; j < 4; j++) {
			modes[j] = {
				.width = connector_info->modes[j].hactive,
				.height = connector_info->modes[j].vactive,
				.pitch = align_up(connector_info->modes[j].hactive * (32 / 8), 64),
				.bpp = 32
			};
		}

		connector = _connector;

		goto found_connector;
	}

found_connector:
	if (!connector) {
		debugf("INTEL: No connector found\n");
		return;
	}

	set_mode(context, connector, connector_info, 0, &modes[0]);
}

void init() {
	pci::register_pci_driver(0x8086, 0x0166, intel_gpu); // Intel 3rd Gen Core GPU (Ivy Bridge)
	pci::register_pci_driver(0x8086, 0x5917, intel_gpu); // Intel 8th Gen Core GPU (Coffee Lake)
}

define_module("intel_gpu", init, null_ptr_func, null_ptr_func);