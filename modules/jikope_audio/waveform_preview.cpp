#include "waveform_preview.h"
#include "scene/main/node.h"

static const Color BG_COLOR = Color::hex(0x262623FF);

void WaveformPreview::_notification(int p_what) {
	switch (p_what) {
	case NOTIFICATION_ENTER_TREE:
		print_line("Entering");
		break;
	case NOTIFICATION_RESIZED:
		queue_redraw();
		break;
	case NOTIFICATION_DRAW:
		_draw_internal();
		break;
	}
}

void WaveformPreview::_draw_internal() {
	Size2 size = get_size();
	int width = size.width;
	int height = size.height;

	draw_rect(Rect2(0, 0, width, height), BG_COLOR, true, 5);
}

void WaveformPreview::_bind_methods() {

}

WaveformPreview::WaveformPreview() {

}
