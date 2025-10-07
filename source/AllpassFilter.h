//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#pragma once
#include <math.h>
#include <assert.h>
#include "CirculateHelpers.h"
#define E_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062


/// <summary>
/// Allpass filter constructed from TPT State Variable filter taps, as described by Vadim Zavalishin
/// in The Art of VA Filter Design Rev2 (2018)
/// 
/// Reads coefficients from a member pointer to a AllpassState object. 
/// Allowing per sample operation for multiple filters without setter calls on each F or Q update
/// (The AllpassState object containing coefficients need only be updated once regardless 
/// of filter number)
/// 
/// 
/// </summary>
class AllpassFilter {

public:
	struct AllpassInfo {
		double k = 0;
		double g = 0;

		double k_target = 0;
		double g_target = 0;
		double smoothFactor = 0.001;

		void setSmoothTime(double smooth_ms, int sample_rate) {
			double smooth_s = smooth_ms / 1000.0f;
			smoothFactor = 1.0 - exp(-1.0 / ( smooth_s * sample_rate));
		}

	};

	void setSampleRateBlockSize(HELPERS::SetupInfo Setup) {
		sampleRate = Setup.sampleRate;
	}
	/// <summary>
	/// Get a block of samples unsmoothed
	/// </summary>
	/// <param name="inBuffer"></param>
	/// <param name="outBuffer"></param>
	/// <param name="Info"></param>
	void getBlock(float* inBuffer, float* outBuffer, HELPERS::ProcessInfo Info) {

		for (int i = 0; i < Info.numSamples; i++) {
			int index = i + Info.startIndex;
			outBuffer[index] = getNext(inBuffer[index]);
		}
	}

	/// <summary>
	/// A static helper function to calculate coefficients and store them
	/// in the State pointer used by the filters
	/// 
	/// </summary>
	/// <param name="freqHz"></param>
	/// <param name="q"> Normalised 0 to 1, internally clamped</param>

	static void calculateCoefficients(double freq_hz, double q, int sample_rate, AllpassInfo& State) {
		
		double q_actual = 0.5f + (q * 9.5f);

		// Calculate SVF coefficients, and set as targets
		State.g_target = ((E_PI * freq_hz) / (double)sample_rate);
		// BLT warp
		State.g_target = tan(State.g_target);
		State.k_target = 1.0 / (2.0 * q_actual);

		//Smooth				
		double diff_k = State.k_target - State.k;
		double diff_g = State.g_target - State.g;

		State.k += diff_k * State.smoothFactor;
		State.g += diff_g * State.smoothFactor;

		if (abs(diff_k) < 1e-10) State.k = State.k_target;
		if (abs(diff_g) < 1e-10) State.g = State.g_target;

	}

	/// <summary>
	/// Update Filter's member pointer to coefficients
	/// </summary>
	/// <param name="NewState"></param>
	void setStatePointer(AllpassInfo* NewState) {
		State = NewState;
	}
	/// <summary>
	/// Reset memory of filter
	/// </summary>
	void resetState() {
		s1 = 0;
		s2 = 0;

	}
	/// <summary>
	/// Get next processed sample
	/// Coefficients are fetched internally directly from the AllpassState pointer
	/// Set with setStatePointer
	/// </summary>
	/// <param name="x"></param>
	/// <returns></returns>
	float getNext(float x) {
		assert(State);

		double g = State->g;
		double R = State->k;
		double d = 1.0 / (1.0 + 2 * R * g + pow(g, 2.0));
		double BP = (g * (x - s2) + s1) * d;

		// Store old states
		double old_s1 = s1;
		double old_s2 = s2;

		// Update states using old values
		double BP2 = BP + BP;
		s1 = BP2 - old_s1;
		s2 = old_s2 + g * BP2; 

		// denormal protection
		if (std::abs(s1) < 1e-20) s1 = 0.0;
		if (std::abs(s2) < 1e-20) s2 = 0.0;

		return x - 4 * R * BP;
	}


private:
	// Memory
	double s1 = 0;
	double s2 = 0;

	int sampleRate = 0;

	AllpassInfo* State = nullptr;
};