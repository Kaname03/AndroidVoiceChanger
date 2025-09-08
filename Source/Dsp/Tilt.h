#pragma once
#include <JuceHeader.h>

/** 太さ "Thickness"：スペクトル傾斜（チルトEQ）。pivot付近を中心にLow/Highを逆相リンク。 */
class TiltEQ
{
public:
    void prepare (double sampleRate, int channels)
    {
        sr = sampleRate;
        ch = juce::jlimit (1, 2, channels);
        update (0.0f, 1000.0f);
    }

    void update (float tiltDb, float pivotHz)
    {
        tilt = juce::jlimit (-12.0f, 12.0f, tiltDb);
        pivot = juce::jlimit (200.0f, 4000.0f, pivotHz);
        const float lowGain  = +tilt / 2.0f;
        const float highGain = -tilt / 2.0f;

        auto ls = juce::dsp::IIR::Coefficients<float>::makeLowShelf (sr, pivot / juce::MathConstants<float>::sqrt2, 0.707f,juce::Decibels::decibelsToGain (lowGain));
        auto hs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sr, pivot * juce::MathConstants<float>::sqrt2, 0.707f,juce::Decibels::decibelsToGain (highGain));
        *low[0].coefficients = *ls; *low[1].coefficients = *ls;
        *high[0].coefficients = *hs; *high[1].coefficients = *hs;
    }

    void process (juce::AudioBuffer<float>& buf)
    {
        for (int c=0; c<juce::jmin (2, buf.getNumChannels()); ++c)
        {
            low[c].processSamples (buf.getWritePointer (c), buf.getNumSamples());
            high[c].processSamples (buf.getWritePointer (c), buf.getNumSamples());
        }
    }

private:
    double sr = 48000.0;
    int ch = 2;
    float tilt = 0.0f, pivot = 1000.0f;
    juce::dsp::IIR::Filter<float> low[2], high[2];
};
