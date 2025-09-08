#pragma once
#include <JuceHeader.h>

/**
 * 低遅延・外部ライブラリ不要の簡易グラニュラー式ピッチシフター（モノ/ステレオ対応）
 * - 固定長のリングバッファに入力を積み、読み出し速度をpitch比率で変える
 * - 2ボイスの交互クロスフェード（ハン窓）で継ぎ目を軽減
 * - レイテンシはバッファ長相当（デフォ約20ms前後）
 */
class GranularPitchShifter
{
public:
    void prepare (double sampleRate, int channels, int maxBlock)
    {
        sr = sampleRate;
        ch = juce::jlimit (1, 2, channels);
        const int maxDelayMs = 40; // レイテンシ/音質の妥協点
        capacity = juce::nextPowerOfTwo (int (sr * maxDelayMs / 1000.0)) + maxBlock;
        ring.setSize (ch, capacity);
        ring.clear();
        writePos = 0;

        // 粒長（ms）とオーバーラップ
        setGrainMs (18.0); // 15-30ms 推奨
        window.makeHannWindow (grainLength);
        otherWindow.makeHannWindow (grainLength);

        // 読み出しフェーズ初期化
        for (int v=0; v<2; ++v)
            for (int c=0; c<2; ++c)
                phase[v][c] = 0.0;

        setSemitone (0.0f);
    }

    void setGrainMs (double ms)
    {
        grainLength = juce::jmax (32, int (sr * ms / 1000.0));
        fadeLength  = juce::jmax (8, grainLength / 4);
    }

    void setSemitone (float semi)
    {
        ratio = std::pow (2.0, semi / 12.0); // >1 = 上げ, <1 = 下げ
    }

    void reset()
    {
        ring.clear();
        writePos = 0;
        for (int v=0; v<2; ++v)
            for (int c=0; c<2; ++c)
                phase[v][c] = 0.0;
    }

    void process (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out)
    {
        const int n = in.getNumSamples();
        out.setSize (ch, n, false, false, true);
        out.clear();

        // 入力をリングへ
        for (int c=0; c<ch; ++c)
        {
            auto* src = in.getReadPointer (c);
            auto* dst = ring.getWritePointer (c);
            for (int i=0; i<n; ++i)
            {
                dst[(writePos + i) & (capacity - 1)] = src[i];
            }
        }
        writePos = (writePos + n) & (capacity - 1);

        // 2ボイスで読み出してミックス
        juce::HeapBlock<float> temp (size_t (n));
        for (int v=0; v<2; ++v)
        {
            const double offset = (v == 0 ? 0.0 : grainLength * 0.5); // ずらして重ねる
            for (int c=0; c<ch; ++c)
            {
                auto* dst = out.getWritePointer (c);
                for (int i=0; i<n; ++i)
                {
                    const double ph = phase[v][c] + i * ratio;
                    const int    rIndex = (writePos - n + i - int (offset)) & (capacity - 1);
                    const double fracPos = ph;

                    // 線形補間でサンプル取得
                    const int idx0 = (rIndex - int (fracPos)) & (capacity - 1);
                    const int idx1 = (idx0 - 1) & (capacity - 1);
                    const float x0 = ring.getSample (c, idx0);
                    const float x1 = ring.getSample (c, idx1);
                    const float frac = float (fracPos - std::floor (fracPos));
                    const float s = x0 * (1.0f - frac) + x1 * frac;

                    // 窓（ハン）でフェード
                    const int local = (int) (std::fmod (ph, (double) grainLength));
                    const float w = windowValue (local);

                    dst[i] += s * w * 0.5f; // 2ボイス合算で0.5
                }
                phase[v][c] += ratio * n;
                // フェーズが粒長を超えたら折り返して継ぎ目を避ける
                const double mod = std::fmod (phase[v][c], (double) grainLength);
                if (mod < 0) phase[v][c] += grainLength;
                else phase[v][c] = mod;
            }
        }
    }

private:
    float windowValue (int local) const
    {
        // 簡易ハン窓
        const double t = juce::jlimit (0, grainLength-1, local) / (double) (grainLength-1);
        return float (0.5 * (1.0 - std::cos (juce::MathConstants<double>::twoPi * t)));
    }

    double sr = 48000.0;
    int ch = 2;
    int capacity = 1 << 12;
    int writePos = 0;

    int grainLength = 1024, fadeLength = 128;
    juce::dsp::WindowingFunction<float> window { 1024, juce::dsp::WindowingFunction<float>::hann, false };
    juce::dsp::WindowingFunction<float> otherWindow { 1024, juce::dsp::WindowingFunction<float>::hann, false };

    juce::AudioBuffer<float> ring;
    double phase[2][2] {{0,0},{0,0}};
    double ratio = 1.0;
};
