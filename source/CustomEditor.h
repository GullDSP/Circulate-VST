#pragma once

#include "vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "vstgui/uidescription/uidescription.h"
#include "vstgui/uidescription/icontroller.h"
#include "vstgui/uidescription/uiviewswitchcontainer.h"
#include "vstgui/uidescription/uiattributes.h"

/// <summary>
/// Custom editor, most of this is just manually switching the views for the center control
/// as the viewswitchcontainer was buggy in ableton
/// </summary>
class CustomEditor : public VSTGUI::VST3Editor {

public:
	CustomEditor(Steinberg::Vst::EditController* controller, VSTGUI::UTF8StringPtr templatename,
		VSTGUI::UTF8StringPtr xml) : VST3Editor(controller, templatename, xml) {

		std::vector<double> zoomFactors = { 0.5,1,1.5,2,3,4,8, 16 };
		VST3Editor::setAllowedZoomFactors(zoomFactors);
	
	}
	void setZoomFactor(double factor)  {
		currentZoomFactor = factor;
		VST3Editor::setZoomFactor(factor);
	}

	float getZoomFactor() {
		return currentZoomFactor;
	}

	void setSwitchToHz() {
		switchIsHz = true;
		if (haveRqdPointers) {
			pHzContainer->setVisible(true);
			pNoteContainer->setVisible(false);
		}
	}
	void setSwitchToNote() {
		switchIsHz = false;
		if (haveRqdPointers) {
			pHzContainer->setVisible(false);
			pNoteContainer->setVisible(true);
		}
	}

	void close() override {

		pNoteContainer = nullptr;
		pHzContainer = nullptr;

		switchIsHz = true;
		haveRqdPointers = false;

		VST3Editor::close();
	}

	void valueChanged(VSTGUI::CControl* pControl) override {

		if (!pControl) return;


		int tag = pControl->getTag();
		if ((tag == CIRCULATE_PARAMS::kSetSwitch) && haveRqdPointers) {

			switchIsHz = !switchIsHz;
			pHzContainer->setVisible(switchIsHz);
			pNoteContainer->setVisible(!switchIsHz);


		}

		VST3Editor::valueChanged(pControl);

	}



	VSTGUI::CView* verifyView(VSTGUI::CView* view, const VSTGUI::UIAttributes& attributes, const VSTGUI::IUIDescription* description) override {
	

		if (auto name = attributes.getAttributeValue("name"))
		{
			if (*name == "HzControl")
			{
				pHzContainer = dynamic_cast<VSTGUI::CViewContainer*> (view);
				if (pHzContainer) pHzContainer->setVisible(switchIsHz);
			}
			else if (*name == "NoteControl")
			{
				pNoteContainer = dynamic_cast<VSTGUI::CViewContainer*> (view);
				if (pNoteContainer) pNoteContainer->setVisible(!switchIsHz);
			}
		}

		if (pHzContainer && pNoteContainer) haveRqdPointers = true;

		return VST3Editor::verifyView(view, attributes, description);
	};

private:
	float currentZoomFactor = 1.0;
	VSTGUI::CViewContainer* pNoteContainer = nullptr;
	VSTGUI::CViewContainer* pHzContainer = nullptr;

	bool switchIsHz = true;
	bool haveRqdPointers = false;
};