//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "base/source/fstreamer.h"
#include "CustomEditor.h"

#define MAX_ZOOM_FACTOR_LIMIT 16
#define MIN_ZOOM_FACTOR_LIMIT 0.1

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
	

	double depth, center, note, focus, type, offset, bypass, feed, spread;

	// Read values in the SAME ORDER the processor wrote them
	if (streamer.readDouble(depth) == false) return kResultFalse;
	if (streamer.readDouble(center) == false) return kResultFalse;
	if (streamer.readDouble(note) == false) return kResultFalse;
	if (streamer.readDouble(focus) == false) return kResultFalse;
	if (streamer.readDouble(type) == false) return kResultFalse;
	if (streamer.readDouble(offset) == false) return kResultFalse;
	if (streamer.readDouble(bypass) == false) return kResultFalse;
	if (streamer.readDouble(feed) == false) return kResultFalse;
	if (streamer.readDouble(spread) == false) spread = 0.0; // if old preset set spread to zero
	
	// Update the controller's parameter objects.
	setParamNormalized(CIRCULATE_PARAMS::kDepth, depth);
	setParamNormalized(CIRCULATE_PARAMS::kCenter, center);
	setParamNormalized(CIRCULATE_PARAMS::kCenterST, note);
	setParamNormalized(CIRCULATE_PARAMS::kFocus, focus);
	setParamNormalized(CIRCULATE_PARAMS::kSetSwitch, type);
	setParamNormalized(CIRCULATE_PARAMS::kNoteOffset, offset);
	setParamNormalized(CIRCULATE_PARAMS::kBypass, bypass);
	setParamNormalized(CIRCULATE_PARAMS::kFeed, feed);
	setParamNormalized(CIRCULATE_PARAMS::kSpread, spread);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::setState (IBStream* state)
{
	if (state) {
		Steinberg::IBStreamer streamer(state, kLittleEndian);
		int id = -1;
		double value = 0;
		if (streamer.readInt32(id) && streamer.readDouble(value)) {

			if (id == kZoomFactorID) {

				if ((value > MIN_ZOOM_FACTOR_LIMIT) && (value < MAX_ZOOM_FACTOR_LIMIT)) {
					currentZoomFactor = value;
					if (currentEditor) 	currentEditor->setZoomFactor(currentZoomFactor);

				}

			}

		}


	}

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateController::getState (IBStream* state)
{
	if (state) {

		currentZoomFactor = 1.0f;
		if (currentEditor) {

			if (currentEditor = static_cast<CustomEditor*>(currentEditor)) {
				currentZoomFactor = currentEditor->getZoomFactor();

			};
		}

		Steinberg::IBStreamer streamer(state, kLittleEndian);
		streamer.writeInt32(kZoomFactorID);
		streamer.writeDouble(currentZoomFactor);

	}

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API CirculateController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new CustomEditor (this, "view", "editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace CirculateVST
