//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include <vector>
namespace HELPERS {
	#define MAX_NOTE_NUM 128 
	#define MAX_FREQ_HZ 18000
	#define MIN_FREQ_HZ 20
	#define MAX_NUM_STAGES 64
	/// <summary>
	/// Container for info about the current setup
	/// </summary>
	struct SetupInfo {
		int blockSize = 0;
		int sampleRate = 0;
	};
	/// <summary>
	/// Container with info about the current process call
	/// </summary>
	struct ProcessInfo {
		int numSamples = 0;
		int startIndex = 0;
	};

	typedef std::vector<float> AudioBuffer;

	inline double noteNumToHz(int note_num) {
		return 440.0 * pow(2.0, (note_num - 69.0) / 12.0);
	}

	inline void mixBuffers(float* out, float* A, float* B, ProcessInfo Info, float mix) {
		for (int i = 0; i < Info.numSamples; i++) {
			int index = Info.startIndex + i;
			out[index] = (mix * A[index]) + ((1.0 - mix) * B[index]);
		}
	}
	/// <summary>
	/// Simple smoother for general purpose smoothing
	/// </summary>
	class ValueSmoother {
		public:
			double getSmoothedValue(double target) {
				double difference = target - lastValue;
				// Snap to target if the difference is very small
				if (std::abs(difference) < 1e-9f) {
					lastValue = target;
				}
				else {
					lastValue += difference * smoothFactor;
				}
				return lastValue;
			}
			void setSmoothTime(double time_ms, int sample_rate) {
				if (time_ms > 0) {
					smoothFactor = 1.0f - expf(-2.0f * 3.141592653589f / (time_ms * 0.001f * sample_rate));
				}
				else {
					smoothFactor = 1.0f; // No smoothing
				}
			}
			void reset() {
				lastValue = 0;
			}

		private:
			double lastValue = 0;
			double smoothFactor = 0.005;
	};
	

}