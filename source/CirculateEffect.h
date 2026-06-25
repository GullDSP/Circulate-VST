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

			float spread_now = pParams->Spread.getSampleAccurateValue(s);
			spread_now = spread_now * spread_now;
			float offset_fact = 0.0;
			float sign = 1.0;
			for (int i = 0; i < mNumActiveStages; i++) {
				// Apply each allpass
				float current_scale = (i + 1.0f) / MAX_NUM_STAGES;

				currentSample = AP[i].getNext(currentSample, 1.0 + (offset_fact * sign) + (jitterOffsets[i] * spread_now * current_scale * 3.5));
				offset_fact += (0.015f * spread_now);
				sign = sign * -1.0;
			}

			// Safety limiter, kicks in only above threshold (abs > 0.99)
			// We safety limit to cover edge cases in extremely fast parameter changes
			// These can cause transient overs due to the old filter state, these overs propagate 
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

	const float jitterOffsets[MAX_NUM_STAGES] = {
	 0.0f, -0.054f,  0.012f, -0.068f,  0.045f, -0.003f,  0.061f, -0.027f,
	-0.019f,  0.038f, -0.042f,  0.005f,  0.057f, -0.011f,  0.024f, -0.063f,
	 0.008f, -0.035f,  0.052f, -0.048f,  0.016f,  0.067f, -0.022f,  0.039f,
	-0.007f,  0.049f, -0.059f,  0.015f, -0.029f,  0.064f, -0.014f,  0.033f,
	 0.001f, -0.044f,  0.028f, -0.009f,  0.055f, -0.037f,  0.021f, -0.066f,
	 0.041f, -0.025f,  0.018f,  0.069f, -0.051f,  0.004f, -0.032f,  0.047f,
	-0.017f,  0.059f, -0.006f,  0.026f, -0.043f,  0.013f, -0.062f,  0.036f,
	-0.028f,  0.053f, -0.010f,  0.044f, -0.056f,  0.002f, -0.039f,  0.070f
	};

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
