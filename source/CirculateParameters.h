//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#pragma once
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "CirculateHelpers.h"
#include "LogRangeParameter.h"
#include <cmath>
#include <vector>
/// <summary>
/// This file contains the parameter system. Including parameter tags, parameter classes 
/// and the registration function to register parameters with the controller
/// </summary>
namespace CIRCULATE_PARAMS {
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
		parameters.addParameter(centerNoteParam);
		
		Steinberg::Vst::StringListParameter* HzSwitch = new Steinberg::Vst::StringListParameter(STR16("SwitchHz"), CirculateParamIDs::kSetSwitch, 0, Steinberg::Vst::ParameterInfo::kIsHidden);
			HzSwitch->appendString(STR16("Hz"));
			HzSwitch->appendString(STR16("ST"));
		
		parameters.addParameter(HzSwitch);

		// Center Hz param
	
		auto* centerHzParam = new LogRangeParameter(
			STR16("Frequency"),
			CIRCULATE_PARAMS::kCenter,
			MIN_FREQ_HZ,
			MAX_FREQ_HZ,
			0.5,
			nullptr,
			Steinberg::Vst::ParameterInfo::kCanAutomate
		
		);
		
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
			32,                           
			0, // Zero steps, we don't need the steps internally (it is cast to Int)
			Steinberg::Vst::ParameterInfo::kNoFlags
		);
		depthParam->setPrecision(0);

		int flags = Steinberg::Vst::ParameterInfo::kCanAutomate;

		parameters.addParameter(depthParam);
		parameters.addParameter(STR16("Bypass"), STR16(""), 1, 0, Steinberg::Vst::ParameterInfo::kIsBypass, CirculateParamIDs::kBypass);
		parameters.addParameter(STR16("Focus"), STR16(""), 0, 0.5, flags, CirculateParamIDs::kFocus);
		parameters.addParameter(STR16("Feedback"), STR16(""), 0, 0.5, flags, CirculateParamIDs::kFeed);

	}
	/// <summary>
	/// A single parameter, each with their own
	/// vector to hold parameter changes for a block
	/// </summary>
	class ParamUnit {
	public:
		ParamUnit(int paramID = 0, float default_value = 0, bool sampleAccurate = false, int hostBlockSize = 0) {
			value = default_value;
			id = paramID;
			smoothedValue = default_value;
			blockSize = hostBlockSize;
			if (sampleAccurate) {
				BlockValues.resize(hostBlockSize, default_value);

			}

		}
		/// <summary>
		/// Set parameters value and set dirty
		/// </summary>
		/// <param name="value"></param>
		void set(float value) {
			this->value = value;
			setDirty();
		}
		/// <summary>
		/// Get parameters current value
		/// smoothed
		/// </summary>
		/// <returns></returns>
		float getSmoothed() {

			float difference = value - smoothedValue;

			if (std::abs(difference) < 0.001f)
			{
				smoothedValue = value;
			}
			else
			{
				
				smoothedValue += difference * smoothFactor;
			}

			return smoothedValue;
		}
		float getSampleAccurateSmoothed(int index) {
			value = BlockValues[index];

			return getSmoothed();
		}
		int getID() const {
			return id;
		}
		void setSmoothTime(float timeInMs, float sampleRate) {
			if (timeInMs > 0) {
				smoothFactor = 1.0f - expf(-2.0f * 3.141592653589f / (timeInMs * 0.001f * sampleRate));
			}
			else {
				smoothFactor = 1.0f; // No smoothing
				wantsSmoothing = false;
			}
		}
		float getSampleAccurateValue(int s) {
			return BlockValues[s];
		}

		/// <summary>
		/// Get unsmoothed value and set clean
		/// </summary>
		/// <returns></returns>
		float getExplicit() {

			return value;
		}
		float getLastValue() const {

			if (BlockValues.empty()) {
				return value;
			}

			return BlockValues[blockSize - 1];
		}
		bool isDirty() const {
			return dirty;
		}
		void setDirty() {
			dirty = true;
		}
		void setClean() {
			dirty = false;
		}
		void snapSmoothedValue() {
			smoothedValue = value;
		}
		/// <summary>
		/// Fill the current parameter block with the 
		/// last result from the previous block
		/// </summary>
		void fillWithLastKnown() {
			float lastValue = getLastValue();

			for (int i = 0; i < blockSize; i++) {

				BlockValues[i] = lastValue;

			}

		}
		/// <summary>
		/// Fill the parameter block with an arbitrary value (0...1)
		/// </summary>
		/// <param name="value"></param>
		void fillWith(float value) {

			for (int i = 0; i < blockSize; i++) {

				BlockValues[i] = value;

			}

		}
		float smoothFactor = 0.005;
		std::vector<float> BlockValues;
		bool wantsSmoothing = true;
	private:
		float value = 0;
		float smoothedValue = 0;
		int id = 0;
		bool dirty = false;
		int blockSize = 0;
		
		
	};

	/// <summary>
	/// Container class for parameters.
	///
	/// </summary>
	class AudioEffectParameters {
	public:
		AudioEffectParameters(int blockSize, int sampleRate) :
			Center(kCenter, 0.5, true, blockSize),
			Focus(kFocus, 0.0, true, blockSize),
			Note(kCenterST, 0.5, true, blockSize),
			Depth(kDepth, 0.5, true, blockSize),
			CenterType(kSetSwitch, 0.0, true, blockSize),
			NoteOffset(kNoteOffset, 0.5, true, blockSize),

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
			Depth.setSmoothTime(0, sampleRate);
			CenterType.setSmoothTime(0, sampleRate);
			Note.setSmoothTime(0, sampleRate);
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

		void setAllClean() {
			Center.setClean();
			Focus.setClean();
			Note.setClean();
			Depth.setClean();
			CenterType.setClean();
			NoteOffset.setClean();
			Feedback.setClean();

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

