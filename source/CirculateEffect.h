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
		double nyQuist = (Setup.sampleRate / 2.0f);

		maxAllowedFreq = MAX_FREQ_HZ;
		if (nyQuist < MAX_FREQ_HZ) maxAllowedFreq = nyQuist - 500.0f;

	}
	/// <summary>
	/// Set pointer used to access host/plugin parameters
	/// </summary>
	/// <param name="Parameters"></param>
	void getParams(CIRCULATE_PARAMS::AudioEffectParameters* Parameters) {
		pParams = Parameters;

	}
	void updateParams() {
		// Parameters updated here are fine to be updated per block. Parameters updated using smoothed values
		// are done in the processBlock method, so that new smoothed values can be fetched each sample.

		// Hz/ST Switch
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

		// Apply each allpass stage in series
		// Coefficients are only calculated once for all stages, and stored in FilterState
		// Allpass filters have a member pointer to this, so read the updated values directly
			
		for (int s = 0; s < numSamples; s++) {

			// Get num stages (+0.5 for crude rounding)
			mNumActiveStages = static_cast<int>(pParams->Depth.getSampleAccurateValue(s) * MAX_NUM_STAGES + 0.5);
			
			// Get Frequency
			mCenterHz = updateFrequency(s);
		
			// Get Q
			mFocus = pParams->Focus.getSampleAccurateValue(s);

			// Apply curve to Q, for more precision with lower values, where there is more timbre variation
			mFocus = mFocus * mFocus * mFocus;
			
			// Only need to calculate coefficients once...allpasses already have a member pointer to FilterState 
			// (set in setSampleRateBlockSize)
			AllpassFilter::calculateCoefficients(mCenterHz, mFocus, Setup.sampleRate, FilterState);

			// Scale feedback parameter, 
			double feedback = pParams->Feedback.getSampleAccurateValue(s);

			// Snap feedback to allow easy switching off
			if (abs(feedback - 0.5) < 0.1) {
				feedback = 0.5;
			}

			feedback = (feedback - 0.5) * 1.98f;

			// If no stages are active, don't use feedback
			if (mNumActiveStages == 0) {
				feedback = 0;
			}

			// Safety limit feedback
			currentSample = getLimitedSample(currentSample);
			// Add feedback
			currentSample = inBuffer[s] + (feedback * currentSample);

			// Gain compensation
			currentSample *= sqrtf(1.0f - (abs(feedback) / 1.5f)   );


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

	CIRCULATE_PARAMS::AudioEffectParameters* pParams = nullptr;
	AllpassFilter::AllpassInfo FilterState;

	/// Local pointer to access global allpass state
	AllpassFilter::AllpassInfo* pState = nullptr;
	HELPERS::SetupInfo Setup;

	double mCenterHz = DEFAULT_CENTER;
	double mFocus	= DEFAULT_FOCUS;
	double mNoteNumHz		= 0;
	double mNoteOffsetHz		= 0;
	int	mNumActiveStages	= DEFAULT_DEPTH * MAX_NUM_STAGES;

	bool mUseHzControl	 = true;
	double maxAllowedFreq = 0;
	float currentSample = 0;

	HELPERS::ValueSmoother NoteControlSmoother;

	/// <summary>
	/// Fetches current frequency parameters for this sample, determines whether Hz or note is
	/// being used, and clamps to a safe range 
	/// </summary>
	/// <param name="s"> sample index</param>
	/// <returns></returns>
	double updateFrequency(int s) {
		double freqHz = pParams->Center.getSampleAccurateValue(s);
		if (freqHz < 0.0) {
			freqHz = 0.0;
		};
		if (freqHz > 1.0){
			freqHz = 1.0f;
		};

		freqHz = MIN_FREQ_HZ * std::pow(maxAllowedFreq / MIN_FREQ_HZ, freqHz);

		if (!mUseHzControl) {
			freqHz = pParams->Note.getSampleAccurateValue(s);
			freqHz = HELPERS::noteNumToHz((freqHz * MAX_NOTE_NUM));

			// Smooth Note after fetching Hz. There is no point smoothing the 
			// value before it is converted to Hz
			freqHz = NoteControlSmoother.getSmoothedValue(freqHz);

			mNoteOffsetHz = pParams->NoteOffset.getSampleAccurateValue(s);

			// Scale offset to + = 1 octave
			mNoteOffsetHz = (2.0f * mNoteOffsetHz) - 1;
			freqHz = freqHz * pow(2.0, mNoteOffsetHz);

			// Clamp (to stop offset moving above max)
			if (freqHz > maxAllowedFreq) {
				freqHz = maxAllowedFreq;
			}
			if (freqHz < MIN_FREQ_HZ) {
				freqHz = MIN_FREQ_HZ;
			}

		}

		return freqHz;
	}

};
