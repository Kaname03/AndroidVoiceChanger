#pragma once
#include <JuceHeader.h>

/** 声道長 "Length" を a倍スケーリング（フォルマント移動）するための片方向リサンプラ */
class LengthResampler
{
public:
    void prepare (double sampleRate, int channels)
    {
        sr = sampleRate;
        ch = juce::jlimit (1, 2, channels);
        for (int i=0;i<2;++i) interp[i].reset();
        temp.setSize (ch, 0);
        setRatio (1.0);
    }

    void setRatio (double a)
    {
        ratio = juce::jlimit (0.5, 2.0, a);
    }

    /** in -> 長さ a倍でリサンプル -> out */
    void process (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out)
    {
        const int inN = in.getNumSamples();
        const int outN = int (std::ceil (inN / ratio)) + 8;
        out.setSize (in.getNumChannels(), outN, false, false, true);

        for (int c=0; c<in.getNumChannels(); ++c)
        {
            const float* src = in.getReadPointer (c);
            float* dst = out.getWritePointer (c);
            int produced = interp[c].process (ratio, src, dst, inN);
            // produced で outN とズレたら埋める
            for (int i=produced; i<outN; ++i) dst[i] = 0.0f;
        }
    }

    void reset()
    {
        for (auto& i : interp) i.reset();
    }

private:
    double sr = 48000.0;
    int ch = 2;
    double ratio = 1.0;
    juce::LagrangeInterpolator interp[2];
    juce::AudioBuffer<float> temp;
};
