#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include <rubberband/RubberBandStretcher.h>

#include "../src/rubberbandshifter.hpp"

class RubberBandInitTest : public juce::UnitTest
{
public:
    RubberBandInitTest() : juce::UnitTest("RubberBand", "DSP") {}

    void runTest() override
    {
        beginTest("Direct RubberBand library initialization");
        testDirectRubberBandInit();

        beginTest("RubberBandShifter wrapper initialization");
        testRubberBandShifterInit();

        beginTest("Multiple rapid initializations");
        testMultipleInitsRapidly();
    }

private:
    void testDirectRubberBandInit()
    {
        auto start = juce::Time::getMillisecondCounterHiRes();
        
        try {
            using RBS = RubberBand::RubberBandStretcher;
            unsigned int options = RBS::OptionProcessRealTime;
            
            logMessage("Creating RubberBandStretcher with channels=2, sampleRate=44100...");
            auto stretcher = std::make_unique<RBS>(44100, 2, options);
            
            logMessage("Setting time ratio...");
            stretcher->setTimeRatio(1.0);
            
            logMessage("Setting pitch scale...");
            stretcher->setPitchScale(1.0f);
            
            auto end = juce::Time::getMillisecondCounterHiRes();
            auto duration = end - start;
            logMessage("RubberBand direct init completed in " + juce::String(duration, 1) + "ms");
            
            expect(true, "RubberBand initialization should complete without hanging");
            
        } catch (const std::exception& e) {
            expect(false, "Exception during RubberBand init: " + juce::String(e.what()));
        }
    }

    void testRubberBandShifterInit()
    {
        auto start = juce::Time::getMillisecondCounterHiRes();
        
        try {
            retuner::dsp::RubberBandShifter<float> shifter;
            
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = 44100.0;
            spec.maximumBlockSize = 512;
            spec.numChannels = 2;
            
            logMessage("Calling shifter.prepare()...");
            shifter.prepare(spec);
            
            auto end = juce::Time::getMillisecondCounterHiRes();
            auto duration = end - start;
            logMessage("RubberBandShifter prepare completed in " + juce::String(duration, 1) + "ms");
            
            expect(duration < 1000.0, "RubberBandShifter preparation should complete in under 1 second");
            
        } catch (const std::exception& e) {
            expect(false, "Exception during RubberBandShifter init: " + juce::String(e.what()));
        }
    }

    void testMultipleInitsRapidly()
    {
        for (int i = 0; i < 3; ++i) {
            logMessage("Init attempt " + juce::String(i + 1) + "...");
            
            auto start = juce::Time::getMillisecondCounterHiRes();
            
            try {
                retuner::dsp::RubberBandShifter<float> shifter;
                
                juce::dsp::ProcessSpec spec;
                spec.sampleRate = 44100.0;
                spec.maximumBlockSize = 512;
                spec.numChannels = 2;
                
                shifter.prepare(spec);
                
                auto end = juce::Time::getMillisecondCounterHiRes();
                auto duration = end - start;
                logMessage("Attempt " + juce::String(i + 1) + " took " + juce::String(duration, 1) + "ms");
                
                expect(duration < 1000.0, "Each initialization should complete quickly");
                
            } catch (const std::exception& e) {
                expect(false, "Exception during attempt " + juce::String(i + 1) + ": " + juce::String(e.what()));
            }
            
            // Small delay between attempts
            juce::Thread::sleep(100);
        }
    }
};

static RubberBandInitTest rubberBandInitTest;