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

	void setSwitchToHz(bool isHz) { 
		switchIsHz = isHz; 
		updateViewVisibility();
	
	}
	bool isHzMode() const { return switchIsHz; }

	void updateViewVisibility() {
		if (pNoteContainer && pHzContainer) {
			pHzContainer->setVisible(switchIsHz);
			pNoteContainer->setVisible(!switchIsHz);
		}
	}

	void close() override {

		pNoteContainer = nullptr;
		pHzContainer = nullptr;

		switchIsHz = true;

		VST3Editor::close();
	}

	void valueChanged(VSTGUI::CControl* pControl) override {

		if (!pControl) return;

		int tag = pControl->getTag();
		if ((tag == CIRCULATE_PARAMS::kSetSwitch)) {
			float value = pControl->getValue();
			switchIsHz = (value < 0.5);

			updateViewVisibility();
		}

		VST3Editor::valueChanged(pControl);
	}

	VSTGUI::CView* verifyView(VSTGUI::CView* view, const VSTGUI::UIAttributes& attributes, const VSTGUI::IUIDescription* description) override {
	
		// Get pointers to the two types of center control
		if (auto name = attributes.getAttributeValue("name"))
		{
			if (*name == "HzControl")
			{
				pHzContainer = dynamic_cast<VSTGUI::CViewContainer*> (view);
			}
			else if (*name == "NoteControl")
			{
				pNoteContainer = dynamic_cast<VSTGUI::CViewContainer*> (view);
			}
		}

		updateViewVisibility();

		return VST3Editor::verifyView(view, attributes, description);
	};

private:

	VSTGUI::SharedPointer<VSTGUI::CViewContainer> pNoteContainer = nullptr;
	VSTGUI::SharedPointer<VSTGUI::CViewContainer> pHzContainer = nullptr;
	bool switchIsHz = true;
};