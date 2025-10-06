//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "base/source/fstreamer.h"


using namespace Steinberg;

namespace CirculateVST {
//------------------------------------------------------------------------
// CirculateProcessor
//------------------------------------------------------------------------
CirculateProcessor::CirculateProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kCirculateControllerUID);
}

//------------------------------------------------------------------------
CirculateProcessor::~CirculateProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	return kResultOk;
}

tresult PLUGIN_API CirculateProcessor::setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, int32 numOuts) 
{
	// Only allow mono or stereo, with one bus either way
	if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0] &&
		(inputs[0] == Steinberg::Vst::SpeakerArr::kMono || inputs[0] == Steinberg::Vst::SpeakerArr::kStereo))
	{
		return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}
/// <summary>
/// Get all changes for a parameter and put them in a vector
/// </summary>
/// <param name="queue"></param>
/// <param name="paramID"></param>
/// <param name="blockSize"></param>
void CirculateProcessor::getParamChangesThisBlock(Steinberg::Vst::IParamValueQueue* queue, int paramID, int blockSize) {

	// Get the target parameter value vector from the parameter object
	if (paramID < 1) {
		return;
	}
	CIRCULATE_PARAMS::ParamUnit* Param = Params->getParameter(paramID);
	float* ParamValues = nullptr;
	if (Param) {
		 ParamValues = Param->BlockValues.data();
	}
	else {
		return;
	}
	
	// Iterate over samples, checking if there is a change. Otherwise set as last value
	Steinberg::Vst::ParamValue currentValue = 0;
	Steinberg::Vst::ParamValue nextValue = 0;

	int changeIndex = 0;
	int nextChange = -1;
	int numChanges = queue->getPointCount();

	if (numChanges > 0) {
		queue->getPoint(0, nextChange, nextValue);
	}
	
	// Get last value from previous block
	currentValue = Param->getLastValue();

	for (int i = 0; i < blockSize; i++) {
		


		if (i == nextChange) {

			currentValue = nextValue;
			changeIndex++;

			if (changeIndex < numChanges) {
				queue->getPoint(changeIndex, nextChange, nextValue);
				
			}

		}

		ParamValues[i] = currentValue;
	}
	// Pre Smooth
	for (int i = 0; i < blockSize; i++) {
		Param->BlockValues[i] = Param->getSampleAccurateSmoothed(i);
	}

}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::process (Vst::ProcessData& data)
{

	//////////////////////////////////////////////

	// Pre-Fill param values with last value (to prevent previous
	// block being re-read
	if (Params) {
		for (auto& param : Params->ParameterList) {
			
			param->fillWithLastKnown();
		
		}
	}

	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
			{

				// Check Bypass parameter
				if (paramQueue->getParameterId() == CIRCULATE_PARAMS::kBypass) {
					int offset = 0;
					Steinberg::Vst::ParamValue value = 0;
					int pointCount = paramQueue->getPointCount();
					paramQueue->getPoint(pointCount - 1, offset,  value);

					if (value > 0.5) {
						isBypassed = true;

					}
					else {
						isBypassed = false;
					}

					continue;
				}

				getParamChangesThisBlock(paramQueue, paramQueue->getParameterId(), data.numSamples);
			}
			
		}
	}
	

	if (!data.numInputs) {
		return false;
	}
	if (!data.numOutputs) {
		return false;
	}

	if (data.inputs[0].numChannels != 2) {
		return false;
	}

	if (isBypassed) {
		int numBus = 1;
		int numChan = 2;

		for (int b = 0; b < numBus; b++) {
			for (int c = 0; c < numChan; c++) {
				float* in = data.inputs[b].channelBuffers32[c];
				float* out = data.outputs[b].channelBuffers32[c];

				memcpy(out, in, sizeof(float) * data.numSamples);

			}
		}

		return false;
	}

	if (data.numSamples > 0)
	{
		int numBus = 1;
		int numChan = 2;

		if (isBypassed) {
			for (int b = 0; b < numBus; b++) {
				for (int c = 0; c < numChan; c++) {
					float* in = data.inputs[b].channelBuffers32[c];
					float* out = data.outputs[b].channelBuffers32[c];

					memcpy(out, in, sizeof(float) * data.numSamples);
		

				}
			}
		}

		for (int b = 0; b < numBus; b++) {
			for (int c = 0; c < numChan; c++) {
				float* in = data.inputs[b].channelBuffers32[c];
				float* out = data.outputs[b].channelBuffers32[c];

					AudioEffect[c].getBlock(in, out, data.numSamples);
				
				

			}
		}
		
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	HELPERS::SetupInfo Setup;
	Setup.blockSize = newSetup.maxSamplesPerBlock;
	Setup.sampleRate = newSetup.sampleRate;

	Params = new CIRCULATE_PARAMS::AudioEffectParameters(newSetup.maxSamplesPerBlock, newSetup.sampleRate);

	AudioEffect[0].setSampleRateBlockSize(Setup);
	AudioEffect[1].setSampleRateBlockSize(Setup);

	// Send pointer to params to effect
	AudioEffect[0].getParams(Params);
	AudioEffect[1].getParams(Params);

	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::setState (IBStream* state)
{
	// called when loading a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	

	float depth, center, note, focus, type, offset, bypass, feed;

	// Same order they were written in getState
	if (streamer.readFloat(depth) == false) return kResultFalse;
	if (streamer.readFloat(center) == false) return kResultFalse;
	if (streamer.readFloat(note) == false) return kResultFalse;
	if (streamer.readFloat(focus) == false) return kResultFalse;
	if (streamer.readFloat(type) == false) return kResultFalse;
	if (streamer.readFloat(offset) == false) return kResultFalse;
	if (streamer.readFloat(bypass) == false) return kResultFalse;
	if (streamer.readFloat(feed) == false) return kResultFalse;

	// Fill sample accurate parameter buffers with loaded value
	Params->Depth.fillWith(depth);
	Params->Center.fillWith(center);
	Params->Note.fillWith(note);
	Params->Focus.fillWith(focus);
	Params->CenterType.fillWith(type);
	Params->NoteOffset.fillWith(offset);
	Params->Feedback.fillWith(feed);

	if (bypass > 0.5) {
		isBypassed = true;
	}
	else {
		isBypassed = false;
	}

	// Bypass smoothing for loaded values

	for (auto& param : Params->ParameterList) {
		param->snapSmoothedValue();
		
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::getState (IBStream* state)
{
	if (!Params)
	{
		return kResultOk;
	}
	
	IBStreamer streamer (state, kLittleEndian);

	streamer.writeFloat(Params->Depth.getLastValue());
	streamer.writeFloat(Params->Center.getLastValue());
	streamer.writeFloat(Params->Note.getLastValue());
	streamer.writeFloat(Params->Focus.getLastValue());
	streamer.writeFloat(Params->CenterType.getLastValue());
	streamer.writeFloat(Params->NoteOffset.getLastValue());
	streamer.writeFloat(isBypassed);
	streamer.writeFloat(Params->Feedback.getLastValue());

	return kResultOk;
}


//------------------------------------------------------------------------
} // namespace CirculateVST
