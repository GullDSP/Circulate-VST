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
	if (paramID < 1) {
		return;
	}

	CIRCULATE_PARAMS::ParamUnit* Param = Params->getParameter(paramID);
	double* ParamValues = nullptr;
	if (Param) {
		ParamValues = Param->BlockValues.data();
	}
	else {
		return;
	}

	int numChanges = queue->getPointCount();

	if (numChanges == 0) {
		return; // No changes, already prefilled
	}

	// Get last raw value from previous block
	Steinberg::Vst::ParamValue currentValue = Param->lastExplicit;

	int changeIndex = 0;
	int32 sampleOffset = 0;
	Steinberg::Vst::ParamValue value = 0;

	for (int i = 0; i < blockSize; i++) {
		// Process all changes that occur at or before this sample
		while (changeIndex < numChanges) {
			queue->getPoint(changeIndex, sampleOffset, value);

			if (sampleOffset <= i) {
				currentValue = value;
				changeIndex++;
			}
			else {
				break; // Future change so stop here (if multiple changes this index get all and keep latest)
			}
		}

		ParamValues[i] = currentValue;
	}

	Param->lastExplicit = currentValue;
}
//------------------------------------------------------------------------
tresult PLUGIN_API CirculateProcessor::process (Vst::ProcessData& data)
{

	//////////////////////////////////////////////


	// Pre-Fill param values with last value (to prevent previous
	// block being re-read
	if (Params) {
		Params->setCurrentBlockSizeAndPreFill(data.numSamples);
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

	// Smooth all parameter blocks even if no new changes received, so that smoothing crosses
	// block boundaries
	if (Params) {
		Params->smoothAllParameters();
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


	// Initialise Defaults
	if (Params) {
		Params->Depth.fillWith(DEFAULT_DEPTH);
		Params->Center.fillWith(DEFAULT_CENTER);
		Params->Note.fillWith(DEFAULT_NOTE);
		Params->Focus.fillWith(DEFAULT_FOCUS);
		Params->CenterType.fillWith(DEFAULT_SWITCH);
		Params->NoteOffset.fillWith(DEFAULT_OFFSET);
		Params->Feedback.fillWith(DEFAULT_FEED);
	}

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
	

	double depth, center, note, focus, type, offset, bypass, feed;

	// Same order they were written in getState
	if (streamer.readDouble(depth) == false) return kResultFalse;
	if (streamer.readDouble(center) == false) return kResultFalse;
	if (streamer.readDouble(note) == false) return kResultFalse;
	if (streamer.readDouble(focus) == false) return kResultFalse;
	if (streamer.readDouble(type) == false) return kResultFalse;
	if (streamer.readDouble(offset) == false) return kResultFalse;
	if (streamer.readDouble(bypass) == false) return kResultFalse;
	if (streamer.readDouble(feed) == false) return kResultFalse;

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
	
	streamer.writeDouble(Params->Depth.getLastValue());
	streamer.writeDouble(Params->Center.getLastValue());
	streamer.writeDouble(Params->Note.getLastValue());
	streamer.writeDouble(Params->Focus.getLastValue());
	streamer.writeDouble(Params->CenterType.getLastValue());
	streamer.writeDouble(Params->NoteOffset.getLastValue());
	streamer.writeDouble(isBypassed);
	streamer.writeDouble(Params->Feedback.getLastValue());

	return kResultOk;
}


//------------------------------------------------------------------------
} // namespace CirculateVST
