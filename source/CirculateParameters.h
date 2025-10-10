//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#pragma once
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "CirculateHelpers.h"
#include "LogRangeParameter.h"
#include <cmath>
#include <vector>
/// <summary>
/// This file contains the parameter system. Including parameter tags, parameter classes 
/// and the registration function to register parameters with the controller
/// </summary>
namespace CIRCULATE_PARAMS {

	// DEFAULTS
	#define DEFAULT_CENTER 0.5
	#define DEFAULT_FOCUS 0.5
	#define DEFAULT_DEPTH 0.5
	#define DEFAULT_FEED 0.5
	#define DEFAULT_NOTE 0.5
	#define DEFAULT_OFFSET 0.5
	#define DEFAULT_SWITCH 0.0


	inline const Steinberg::tchar* noteNames[128] = {
		// Octave -1
		STR16("C-1"), STR16("C#-1"), STR16("D-1"), STR16("D#-1"), STR16("E-1"), STR16("F-1"), STR16("F#-1"), STR16("G-1"), STR16("G#-1"), STR16("A-1"), STR16("A#-1"), STR16("B-1"),
		// Octave 0
		STR16("C0"), STR16("C#0"), STR16("D0"), STR16("D#0"), STR16("E0"), STR16("F0"), STR16("F#0"), STR16("G0"), STR16("G#0"), STR16("A0"), STR16("A#0"), STR16("B0"),
		// Octave 1
		STR16("C1"), STR16("C#1"), STR16("D1"), STR16("D#1"), STR16("E1"), STR16("F1"), STR16("F#1"), STR16("G1"), STR16("G#1"), STR16("A1"), STR16("A#1"), STR16("B1"),
		// Octave 2
		STR16("C2"), STR16("C#2"), STR16("D2"), STR16("D#2"), STR16("E2"), STR16("F2"), STR16("F#2"), STR16("G2"), STR16("G#2"), STR16("A2"), STR16("A#2"), STR16("B2"),
		// Octave 3
		STR16("C3"), STR16("C#3"), STR16("D3"), STR16("D#3"), STR16("E3"), STR16("F3"), STR16("F#3"), STR16("G3"), STR16("G#3"), STR16("A3"), STR16("A#3"), STR16("B3"),
		// Octave 4
		STR16("C4"), STR16("C#4"), STR16("D4"), STR16("D#4"), STR16("E4"), STR16("F4"), STR16("F#4"), STR16("G4"), STR16("G#4"), STR16("A4"), STR16("A#4"), STR16("B4"),
		// Octave 5
		STR16("C5"), STR16("C#5"), STR16("D5"), STR16("D#5"), STR16("E5"), STR16("F5"), STR16("F#5"), STR16("G5"), STR16("G#5"), STR16("A5"), STR16("A#5"), STR16("B5"),
		// Octave 6
		STR16("C6"), STR16("C#6"), STR16("D6"), STR16("D#6"), STR16("E6"), STR16("F6"), STR16("F#6"), STR16("G6"), STR16("G#6"), STR16("A6"), STR16("A#6"), STR16("B6"),
		// Octave 7
		STR16("C7"), STR16("C#7"), STR16("D7"), STR16("D#7"), STR16("E7"), STR16("F7"), STR16("F#7"), STR16("G7"), STR16("G#7"), STR16("A7"), STR16("A#7"), STR16("B7"),
		// Octave 8
		STR16("C8"), STR16("C#8"), STR16("D8"), STR16("D#8"), STR16("E8"), STR16("F8"), STR16("F#8"), STR16("G8"), STR16("G#8"), STR16("A8"), STR16("A#8"), STR16("B8"),
		// Octave 9
		STR16("C9"), STR16("C#9"), STR16("D9"), STR16("D#9"), STR16("E9"), STR16("F9"), STR16("F#9"), STR16("G9")
	};

	enum CirculateParamIDs {
		kDepth = 100,
		kCenter,
		kCenterST,
		kFocus,
		kStereo,
		kSetSwitch,
		kNoteOffset,
		kBypass,
		kFeed
		
	};

	inline void registerParameters(Steinberg::Vst::ParameterContainer& parameters) {

		Steinberg::Vst::StringListParameter* centerNoteParam = new Steinberg::Vst::StringListParameter(STR16("Note"), kCenterST);

		for (int i = 0; i < MAX_NOTE_NUM; i++) {
			centerNoteParam->appendString(noteNames[i]);
		}
		centerNoteParam->setNormalized(DEFAULT_NOTE);
		parameters.addParameter(centerNoteParam);
		
		Steinberg::Vst::StringListParameter* HzSwitch = new Steinberg::Vst::StringListParameter(STR16("SwitchHz"), CirculateParamIDs::kSetSwitch, 0, Steinberg::Vst::ParameterInfo::kIsHidden);
			HzSwitch->appendString(STR16("Hz"));
			HzSwitch->appendString(STR16("ST"));
		
		HzSwitch->setNormalized(DEFAULT_SWITCH);
		parameters.addParameter(HzSwitch);


		// Center Hz param
	
		auto* centerHzParam = new LogRangeParameter(
			STR16("Frequency"),
			CIRCULATE_PARAMS::kCenter,
			MIN_FREQ_HZ,
			MAX_FREQ_HZ,
			0.5 ,
			nullptr,
			Steinberg::Vst::ParameterInfo::kCanAutomate
		
		);
		centerHzParam->setNormalized(DEFAULT_CENTER);
		parameters.addParameter(centerHzParam);


		// Note Offset Param
		auto* noteOffset = new Steinberg::Vst::RangeParameter(
			STR16("Fine"),                 
			CirculateParamIDs::kNoteOffset,
			STR16("Oct"),                  
			-1.0,                          
			1.0,                           
			0,                             
			0,                              
			Steinberg::Vst::ParameterInfo::kCanAutomate 
		);
		noteOffset->setPrecision(1);

		parameters.addParameter(noteOffset);

		// Depth Param
		auto* depthParam = new Steinberg::Vst::RangeParameter(
			STR16("Depth"),                   
			CirculateParamIDs::kDepth,             
			STR16("x"),                     
			0,                            
			MAX_NUM_STAGES,                         
			DEFAULT_DEPTH,                           
			0, // Zero steps, we don't need the steps internally (it is cast to Int)
			Steinberg::Vst::ParameterInfo::kNoFlags
		);
		depthParam->setPrecision(0);
		depthParam->setNormalized(DEFAULT_DEPTH);

		parameters.addParameter(depthParam);


		int flags = Steinberg::Vst::ParameterInfo::kCanAutomate;

		parameters.addParameter(STR16("Bypass"), STR16(""), 1, 0, Steinberg::Vst::ParameterInfo::kIsBypass, CirculateParamIDs::kBypass);
		parameters.addParameter(STR16("Focus"), STR16(""), 0, DEFAULT_FOCUS, flags, CirculateParamIDs::kFocus);
		parameters.addParameter(STR16("Feedback"), STR16(""), 0, DEFAULT_FEED, flags, CirculateParamIDs::kFeed);

	}
	/// <summary>
	/// A single parameter, each with their own
	/// vector to hold parameter changes for a block
	/// </summary>
	class ParamUnit {
	public:
		ParamUnit(int paramID = 0, double default_value = 0, bool sampleAccurate = false, int maxBlockSize = 0) {
			value = default_value;
			lastExplicit = default_value;
			smoothedValue = default_value;
			id = paramID;
			smoothedValue = default_value;
			this->maxBlockSize = maxBlockSize;
			this->currentBlockSize = maxBlockSize;

			if (sampleAccurate) {
				BlockValues.resize(maxBlockSize, default_value);

			}



		}
		int getID() {
			return id;
		}
		void setSmoothTime(double timeInMs, int sampleRate) {
			if (timeInMs > 0) {
				smoothFactor = 1.0f - expf(-2.0f * 3.141592653589f / (timeInMs * 0.001f * sampleRate));
			}
			else {
				smoothFactor = 1.0f; // No smoothing
				wantsSmoothing = false;
			}
		}
		double getSampleAccurateValue(int s) {
			return BlockValues[s];
		}
		void smoothBlockValues() {

			// Skip if the parameter doesn't want to be smoothed
			if (!wantsSmoothing) {
				lastValue = lastExplicit;
				smoothedValue = lastExplicit;
				return;
			};
			
			for (int i = 0; i < currentBlockSize; i++) {

				double diff = BlockValues[i] - smoothedValue;
				if (abs(diff) < 1e-3) {
					smoothedValue = BlockValues[i];
					diff = 0;
				}

				smoothedValue += diff * smoothFactor;
				BlockValues[i] = smoothedValue;
				lastValue = smoothedValue;
			}
		}
		double getLastValue() {
			return lastExplicit;
		}
		void fillWithLastKnown() {
			for (int i = 0; i < currentBlockSize; i++) {
				BlockValues[i] = lastExplicit;
			}
		}
		void fillWith(double value) {
			for (int i = 0; i < currentBlockSize; i++) {
				BlockValues[i] = value;
			}

			// Snap smoothing related memory to this value
			// to stop ramping at start of block when smoothed
			lastValue = value;
			smoothedValue = value;
			lastExplicit = value;
		}
		void setCurrentBlockSize(int newSize) {
			currentBlockSize = newSize;

			if (newSize == 0) {
				currentBlockSize = 1;
				BlockValues[0] = lastValue;
				smoothedValue = lastValue;
			}
		}
		std::vector<double> BlockValues;
		bool wantsSmoothing = true;
		double lastExplicit = 0;

	private:

		double value = 0;
		double lastValue = 0;
		double smoothedValue = 0;
		double smoothFactor = 0.005;
		int id = 0;
		int maxBlockSize = 0;
		int currentBlockSize = 0;

	};

	/// <summary>
	/// Container class for parameters.
	///
	/// </summary>
	class AudioEffectParameters {
	public:
		AudioEffectParameters(int blockSize, int sampleRate) :
			Center(kCenter, DEFAULT_CENTER, true, blockSize),
			Focus(kFocus, DEFAULT_FOCUS, true, blockSize),
			Note(kCenterST, DEFAULT_NOTE, true, blockSize),
			Depth(kDepth, DEFAULT_DEPTH, true, blockSize),
			CenterType(kSetSwitch, DEFAULT_SWITCH, true, blockSize),
			NoteOffset(kNoteOffset, DEFAULT_OFFSET, true, blockSize),

			Feedback(kFeed, 0.5, true, blockSize)

		{
			blockSize = blockSize;
			// Add parameter objects to the parameter manager's list
			ParameterList.push_back(&Center);
			ParameterList.push_back(&Focus);
			ParameterList.push_back(&Note);
			ParameterList.push_back(&Depth);
			ParameterList.push_back(&CenterType);
			ParameterList.push_back(&NoteOffset);
			ParameterList.push_back(&Feedback);

			// Initialise smooth times to 20ms (50Hz)
			Center.setSmoothTime(20, sampleRate);
			Focus.setSmoothTime(20, sampleRate);
			NoteOffset.setSmoothTime(20, sampleRate);
			Feedback.setSmoothTime(10, sampleRate);


			// Disable smoothing on discrete parameters
			Depth.setSmoothTime(30, sampleRate);
			CenterType.setSmoothTime(0, sampleRate);
			Note.setSmoothTime(0, sampleRate); // Note is smoothed after conversion to Hz in main loop
		}
		/// <summary>
		/// Return pointer to a parameter unit
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		ParamUnit* getParameter(int id) {

			if (id < 1) return nullptr;

			for (int i = 0; i < ParameterList.size(); i++) {
				if (ParameterList[i]->getID() == id) {
					return ParameterList[i];
				}
			}

			return nullptr;
		}

		void setCurrentBlockSizeAndPreFill(int size) {
			for (auto& param : ParameterList) {

				param->setCurrentBlockSize(size);
				param->fillWithLastKnown();
		
			}

		}
		void smoothAllParameters() {
			for (auto& param : ParameterList) {
				param->smoothBlockValues();
			}
		}

		/// <summary>
/// Get all changes for a parameter and put them in a vector
/// </summary>
/// <param name="queue"></param>
/// <param name="paramID"></param>
/// <param name="blockSize"></param>
		void getParamChangesThisBlock(Steinberg::Vst::IParamValueQueue* queue, int paramID, int blockSize) {
			if (paramID < 1) {
				return;
			}

			CIRCULATE_PARAMS::ParamUnit* Param = getParameter(paramID);
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
			int sampleOffset = 0;
			double value = 0;

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

		//Parameters-----
		ParamUnit Center;
		ParamUnit Focus;
		ParamUnit Note;
		ParamUnit Depth;
		ParamUnit CenterType;
		ParamUnit NoteOffset;
		ParamUnit Feedback;

		std::vector<ParamUnit*> ParameterList;

		int blockSize = 0;

	};

}

