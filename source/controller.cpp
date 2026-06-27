//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "base/source/fstreamer.h"

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
	

	double depth, center, note, focus, type, offset, bypass, feed;

	// Read values in the SAME ORDER the processor wrote them
	if (streamer.readDouble(depth) == false) return kResultFalse;
	if (streamer.readDouble(center) == false) return kResultFalse;
	if (streamer.readDouble(note) == false) return kResultFalse;
	if (streamer.readDouble(focus) == false) return kResultFalse;
	if (streamer.readDouble(type) == false) return kResultFalse;
	if (streamer.readDouble(offset) == false) return kResultFalse;
	if (streamer.readDouble(bypass) == false) return kResultFalse;
	if (streamer.readDouble(feed) == false) return kResultFalse;
	
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

	// Get state of switch
	if (getParamNormalized(CIRCULATE_PARAMS::kSetSwitch) > 0.5) {
		switchIsHzState = false;
	}
	else {
		switchIsHzState = true;
	}

	// Update view anyway, as not saved in state
	if (currentEditor) {
		auto* my_editor = dynamic_cast<CustomEditor*>(currentEditor);
		if (my_editor) {
			my_editor->setSwitchToHz(switchIsHzState);
		}
	}


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

		
		Steinberg::IBStreamer streamer(state, kLittleEndian);
		streamer.writeInt32(kZoomFactorID);
		streamer.writeDouble(currentZoomFactor);
		// don't need to write the switch state as it is stored in the processor

	}

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API CirculateController::createView (FIDString name)
{
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
	
		currentEditor = new CustomEditor (this, "view", "editor.uidesc");

		// Get state of switch
		if (getParamNormalized(CIRCULATE_PARAMS::kSetSwitch) > 0.5) {
			switchIsHzState = false;
		}
		else {
			switchIsHzState = true;
		}

		auto* customEditor = static_cast<CustomEditor*>(currentEditor);
		if (customEditor) {
			// Update editor
			customEditor->setSwitchToHz(switchIsHzState);
			customEditor->setZoomFactor(currentZoomFactor);
			
		}

		return currentEditor;
	}
	return nullptr;
}




//------------------------------------------------------------------------
} // namespace CirculateVST
