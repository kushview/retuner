# Test suite for ReTuner using JUCE UnitTest framework
enable_testing()

add_executable(test_runner)

target_sources(test_runner PRIVATE
    runner.cpp
    ../src/processor.cpp
    ../src/editor.cpp
    ../src/style.cpp
)

target_link_libraries(test_runner PRIVATE
    juce::juce_core
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_product_unlocking
    rubberband
)

target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_compile_definitions(test_runner PRIVATE
    ${RETUNER_JUCE_OPTIONS}
)

add_test(NAME "Units" COMMAND test_runner)
