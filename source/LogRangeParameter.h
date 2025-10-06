
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ustring.h"
#include <cstdlib>
#include <math.h>
/// <summary>
/// Custom override of Parameter class to allow printing of log values
/// For pitch / center
/// </summary>
class LogRangeParameter : public Steinberg::Vst::Parameter
{
public:
    LogRangeParameter(const Steinberg::Vst::TChar* title,
        Steinberg::Vst::ParamID tag,
        Steinberg::Vst::ParamValue minPlain,
        Steinberg::Vst::ParamValue maxPlain,
        Steinberg::Vst::ParamValue defaultValue,
        const Steinberg::Vst::TChar* units = nullptr,
        Steinberg::int32 flags = Steinberg::Vst::ParameterInfo::kCanAutomate)
        : Parameter(title, tag, units, defaultValue, 0, flags, 0)
        , minPlain(minPlain)
        , maxPlain(maxPlain)
    {
        
        double defaultNormalized = toNormalized(defaultValue);
        setNormalized(defaultNormalized);
    }

    Steinberg::Vst::ParamValue toNormalized(Steinberg::Vst::ParamValue plainValue) const SMTG_OVERRIDE
    {
        if (plainValue <= minPlain) return 0.0;
        if (plainValue >= maxPlain) return 1.0;
        return log(plainValue / minPlain) / log(maxPlain / minPlain);
    }

    Steinberg::Vst::ParamValue toPlain(Steinberg::Vst::ParamValue normalizedValue) const SMTG_OVERRIDE
    {
        const double epsilon = 1e-10;

        if (normalizedValue <= epsilon) return minPlain;
        if (normalizedValue >= (1.0 - epsilon)) return maxPlain;
        return minPlain * pow(maxPlain / minPlain, normalizedValue);
    }

    void toString(Steinberg::Vst::ParamValue normalizedValue, Steinberg::Vst::String128 string) const SMTG_OVERRIDE
    {
        Steinberg::Vst::ParamValue plainValue = toPlain(normalizedValue);
        char text[128];
        snprintf(text, sizeof(text), "%.1f Hz", plainValue);

        for (int i = 0; i < 128; ++i) {
            string[i] = text[i];
            if (text[i] == '\0') break;
        }
    }

private:
    Steinberg::Vst::ParamValue minPlain;
    Steinberg::Vst::ParamValue maxPlain;
};