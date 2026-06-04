#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DashboardProfiler
{
public:
    void beginPaint()
    {
        paintStart = juce::Time::getHighResolutionTicks();
    }

    void endPaint()
    {
        auto paintEnd = juce::Time::getHighResolutionTicks();

        auto ms = juce::Time::highResolutionTicksToSeconds(paintEnd - paintStart) * 1000.0;

        frameCount++;
        totalPaintMs += ms;

        if (ms > maxPaintMs)
            maxPaintMs = ms;

        auto now = juce::Time::getMillisecondCounter();

        if (now - lastReportTime >= 1000)
        {
            double avgPaint = frameCount > 0 ? totalPaintMs / frameCount : 0.0;

            DBG("========== UI PROFILER ==========");
            DBG("FPS: " << frameCount);
            DBG("Avg Paint: " << avgPaint << " ms");
            DBG("Max Paint: " << maxPaintMs << " ms");
            DBG("=================================");

            frameCount = 0;
            totalPaintMs = 0.0;
            maxPaintMs = 0.0;
            lastReportTime = now;
        }
    }

private:
    int frameCount = 0;
    double totalPaintMs = 0.0;
    double maxPaintMs = 0.0;
    juce::int64 paintStart = 0;
    uint32_t lastReportTime = juce::Time::getMillisecondCounter();
};
