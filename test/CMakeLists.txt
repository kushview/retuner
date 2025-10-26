# Test suite for ReTuner using JUCE UnitTest framework

# Create test executable
add_executable(test_runner)

# Add sources
target_sources(test_runner PRIVATE
    runner.cpp
    ../src/processor.cpp
    ../src/editor.cpp
    ../src/style.cpp
)

# Link with ReTuner shared code library and required JUCE modules
target_link_libraries(test_runner PRIVATE
    juce::juce_core
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_product_unlocking
    rubberband
)

# Include directories - use CMAKE_CURRENT_SOURCE_DIR for better subproject support
target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Compile definitions - override plugin definitions for tests
target_compile_definitions(test_runner PRIVATE
    ${RETUNER_JUCE_OPTIONS}
)
