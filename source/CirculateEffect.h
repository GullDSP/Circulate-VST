//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include "CirculateHelpers.h"
#include "CirculateParameters.h"
#include "AllpassFilter.h"
#include "Limiter.h"
#include <vector>

class CirculateEffect {
public:
	void setSampleRateBlockSize(HELPERS::SetupInfo Setup) {

		for (int i = 0; i < MAX_NUM_STAGES; i++) {
			AP[i].setSampleRateBlockSize(Setup);
		}
	
		this->Setup = Setup;

		// Set pointer that is passed to filters, so they can directly read updated coefficients
		pState = &FilterState;
		for (int i = 0; i < MAX_NUM_STAGES; i++) {
			// send to each allpass
			AP[i].setStatePointer(pState);

		}

		const double smoothTimeMs = 5;
		// Set coefficient smooth time
		pState->setSmoothTime(smoothTimeMs, Setup.sampleRate);

		// This independent smoother smooths the result of the 
		// note control after converting to Hz (not the note number)
		NoteControlSmoother.setSmoothTime(25, Setup.sampleRate);

		// Calculate max Hz for center frequency
		// Defaulted to 18KHz, cut down for unusually low sample rates
		float nyQuist = (Setup.sampleRate / 2.0f);

		maxAllowedFreq = MAX_FREQ_HZ;
		if (nyQuist < MAX_FREQ_HZ) maxAllowedFreq = nyQuist - 500.0f;

	}
	void getParams(CIRCULATE_PARAMS::AudioEffectParameters* Parameters) {
		pParams = Parameters;

	}
	void updateParams() {
		// Parameters updated here are fine to be updated per block. Parameters updated using smoothed values
		// are done in the processBlock method, so that new smoothed values are fetched each sample.
		// These are however only passed to the AP filters once per block

		// Hz Switch
		float typeNorm = pParams->CenterType.getLastValue();
		if (typeNorm >= 0.5f) {
			mUseHzControl = false;

		}
		else {
			mUseHzControl = true;
		}

	}

	void getBlock(float* inBuffer, float* outBuffer, int numSamples) {
		updateParams();

		if (!inBuffer) {
			return;
		}

		// For each sample, apply each allpass sample by sample. 
		// New smoothed values are fetched every sample and update the filters.
		// This is not so taxing, as the coefficients only need to be calculated once for all filters
		// and the parameter changes for each block are all pre-fetched and smoothed at once to a vector (in Params)
		// Allpasses read directly from the AllpassState FilterState object directly using a member pointer
			
		for (int s = 0; s < numSamples; s++) {

			// Get num stages
			mNumActiveStages = pParams->Depth.getSampleAccurateValue(s) * MAX_NUM_STAGES;
			
			// Get Frequency
			mCenterHz = updateFrequency(s);
		
			// Get Q
			mFocus = pParams->Focus.getSampleAccurateValue(s);

			// Apply curve to Q, for more precision with lower values
			mFocus = mFocus * mFocus * mFocus;
			
			// Only need to calculate coefficients once...allpasses already have a pointer to FilterState 
			// (passed in setSampleRateBlockSize)
			AllpassFilter::calculateCoefficients(mCenterHz, mFocus, Setup.sampleRate, FilterState);

			// Scale feedback parameter, 
			float feedback = pParams->Feedback.getSampleAccurateValue(s);

			// Snap feedback to allow easy switching off
			if (abs(feedback - 0.5) < 0.1) {
				feedback = 0.5;
			}

			feedback = (feedback - 0.5) * 1.98f;

			if (mNumActiveStages == 0) {
				feedback = 0;
			}

			// Safety limit feedback
			currentSample = getLimitedSample(currentSample);
			// Add feedback, and compensate gain
			currentSample = inBuffer[s] + (feedback * currentSample);
			currentSample *= sqrt(1.5f - abs(feedback));

			for (int i = 0; i < mNumActiveStages; i++) {
				// Apply each allpass
				currentSample = AP[i].getNext(currentSample);

			}

			// Safety limiter, kicks in only above threshold (abs > 0.99)
			// We safety limit to cover edge cases in extremely fast parameter changes
			// These can cause overs due to the old filter state, these overs propagate 
			// and are amplified by multiple stages. This limiter prevents these overs.
			currentSample = getLimitedSample(currentSample);

			outBuffer[s] = currentSample;

		}

	}
private:
	AllpassFilter AP[MAX_NUM_STAGES];

	CIRCULATE_PARAMS::AudioEffectParameters* pParams;
	AllpassFilter::AllpassInfo FilterState;

	/// Local pointer to access global allpass state
	AllpassFilter::AllpassInfo* pState = nullptr;
	HELPERS::SetupInfo Setup;

	float mCenterHz = 0;
	float mFocus	= 0;
	float mNoteNumHz		= 0;
	float mNoteOffsetHz		= 0;
	int	mNumActiveStages	= 32;

	bool mUseHzControl	 = true;
	float maxAllowedFreq = 0;
	float currentSample = 0;

	HELPERS::ValueSmoother NoteControlSmoother;

	/// <summary>
	/// Fetches current frequency parameters for this sample, determines whether Hz or note is
	/// being used, and clamps to a safe range 
	/// </summary>
	/// <param name="s"> sample index</param>
	/// <returns></returns>
	float updateFrequency(int s) {
		float freqHz = pParams->Center.getSampleAccurateValue(s);
		if (freqHz < 0.0) {
			freqHz = 0.0;
		};
		if (freqHz > 1.0){
			freqHz = 1.0f;
		};

		freqHz = MIN_FREQ_HZ * std::pow(maxAllowedFreq / MIN_FREQ_HZ, freqHz);

		if (!mUseHzControl) {
			freqHz = pParams->Note.getSampleAccurateValue(s);
			freqHz = HELPERS::noteNumToHz(freqHz * MAX_NOTE_NUM);

			// Smooth Note after fetching Hz. There is no point smoothing the 
			// value before it is converted to Hz
			freqHz = NoteControlSmoother.getSmoothedValue(freqHz);

			mNoteOffsetHz = pParams->NoteOffset.getSampleAccurateValue(s);

			// Scale offset to + = 1 octave
			mNoteOffsetHz = (2.0f * mNoteOffsetHz) - 1;
			freqHz = freqHz * powf(2, mNoteOffsetHz);
		}

		return freqHz;
	}

};
