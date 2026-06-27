//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "CirculateParameters.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "CustomEditor.h"


namespace CirculateVST {

//------------------------------------------------------------------------
//  CirculateController
//------------------------------------------------------------------------
class CirculateController : public Steinberg::Vst::EditControllerEx1, public VSTGUI::VST3EditorDelegate
{
public:
//------------------------------------------------------------------------
	CirculateController () = default;
	~CirculateController () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{

		return (Steinberg::Vst::IEditController*)new CirculateController;
	}

	//--- from IPluginBase -----------------------------------------------
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//--- from EditController --------------------------------------------
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

    // ... (at the end of your source/controller.cpp file) ...
//------------------------------------------------------------------------

		/// to track current zoom
	void onZoomChanged(VSTGUI::VST3Editor* editor, double newZoom) override {
		currentZoomFactor = newZoom;

	};
	
	// called when plugin window closed
	void willClose(VSTGUI::VST3Editor* editor) override {
		auto custom_editor = dynamic_cast<CustomEditor*>(editor);
		if (custom_editor) {
			// Save the switch state before closing
			switchIsHzState = custom_editor->isHzMode();
		}

		currentEditor = nullptr;
	}

 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:
	float currentZoomFactor = 1.0;
	const int kZoomFactorID = 201;

	bool switchIsHzState = true;

	VSTGUI::VST3Editor* currentEditor = nullptr;
};

//------------------------------------------------------------------------
} // namespace CirculateVST
