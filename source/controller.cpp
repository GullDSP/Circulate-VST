//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"

using namespace Steinberg;

namespace CirculateVST {

//------------------------------------------------------------------------
// CirculateController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	CIRCULATE_PARAMS::registerParameters(parameters);

	setKnobMode(Steinberg::Vst::KnobModes::kLinearMode);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);

	float depth, center, note, focus, type, offset, bypass, feed;

	// Read values in the SAME ORDER the processor wrote them
	if (streamer.readFloat(depth) == false) return kResultFalse;
	if (streamer.readFloat(center) == false) return kResultFalse;
	if (streamer.readFloat(note) == false) return kResultFalse;
	if (streamer.readFloat(focus) == false) return kResultFalse;
	if (streamer.readFloat(type) == false) return kResultFalse;
	if (streamer.readFloat(offset) == false) return kResultFalse;
	if (streamer.readFloat(bypass) == false) return kResultFalse;
	if (streamer.readFloat(feed) == false) return kResultFalse;

	// Update the controller's parameter objects.
	setParamNormalized(CIRCULATE_PARAMS::kDepth, depth);
	setParamNormalized(CIRCULATE_PARAMS::kCenter, center);
	setParamNormalized(CIRCULATE_PARAMS::kCenterST, note);
	setParamNormalized(CIRCULATE_PARAMS::kFocus, focus);
	setParamNormalized(CIRCULATE_PARAMS::kSetSwitch, type);
	setParamNormalized(CIRCULATE_PARAMS::kNoteOffset, offset);
	setParamNormalized(CIRCULATE_PARAMS::kBypass, bypass);
	setParamNormalized(CIRCULATE_PARAMS::kFeed, feed);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API CirculateController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace CirculateVST
