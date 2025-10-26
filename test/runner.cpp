#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "rubberbandtest.cpp"

//==============================================================================
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    
    std::cout << "ReTuner JUCE Unit Tests" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << std::endl;
    
    // Create and run all tests
    juce::UnitTestRunner testRunner;
    testRunner.runAllTests();
    
    std::cout << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "=============" << std::endl;
    
    // Print results
    int totalFailures = 0;
    
    for (int i = 0; i < testRunner.getNumResults(); ++i)
    {
        auto* result = testRunner.getResult(i);
        totalFailures += result->failures;
        
        juce::String status = result->failures == 0 ? "PASSED" : "FAILED";
        std::cout << status.toStdString() << ": " << result->unitTestName.toStdString() 
                  << " (" << result->failures << " failures)" << std::endl;
    }
    
    std::cout << std::endl;
    
    if (totalFailures == 0)
    {
        std::cout << "✓ All tests passed!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "✗ " << totalFailures << " test(s) failed!" << std::endl;
        return 1;
    }
}