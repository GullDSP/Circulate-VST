#pragma once

#include "vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "vstgui/uidescription/uidescription.h"
#include "vstgui/uidescription/icontroller.h"


class CustomEditor : public VSTGUI::VST3Editor {

public:
	CustomEditor(Steinberg::Vst::EditController* controller, VSTGUI::UTF8StringPtr templatename,
		VSTGUI::UTF8StringPtr xml) : VST3Editor(controller, templatename, xml) {

		std::vector<double> zoomFactors = { 0.5,1,1.5,2,3,4,8 };
		VST3Editor::setAllowedZoomFactors(zoomFactors);

	}


private:


};