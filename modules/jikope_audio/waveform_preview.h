#ifndef WAVEFORM_PREVIEW_H
#define WAVEFORM_PREVIEW_H

#include "scene/gui/control.h"


class WaveformPreview : public Control
{
	GDCLASS(WaveformPreview, Control)
public:
    WaveformPreview();
private:
	void _draw_internal();

	// Ref<AudioStreamOGG

protected:
	void _notification(int p_what);
	static void _bind_methods();
};


#endif /* WAVEFORM_PREVIEW_H */
