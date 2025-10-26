// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <type_traits>
#include <vector>

namespace retuner {

struct TuningStandard {
    enum {
        Baroque = 0, // ~A4 = 415 Hz, Baroque era pitch
        Classical,   // ~A4 = 430 Hz, Classical/early Romantic
        Verdi,       // A4 = 432 Hz, aka "Verdi" or "Scientific" pitch
        Modern,      // A4 = 440 Hz, current ISO standard
        High,        // A4 = 444 Hz, bright modern tuning
        Total
    };
};

template <typename Int>
inline static const char* tuningStandardString (Int standard)
{
    static_assert (std::is_integral_v<Int>, "Template parameter must be an integer type");
    switch (static_cast<int> (standard)) {
        case (int) TuningStandard::Baroque:
            return "Baroque (415 Hz)";
            break;
        case (int) TuningStandard::Classical:
            return "Classical (430 Hz)";
            break;
        case (int) TuningStandard::Verdi:
            return "Verdi (432 Hz)";
            break;
        case (int) TuningStandard::Modern:
            return "Modern (440 Hz)";
            break;
        case (int) TuningStandard::High:
            return "High (444 Hz)";
            break;
    }

    return "Unknown";
}

template <typename Int>
inline static float tuningStandardFrequency (Int standard)
{
    static_assert (std::is_integral_v<Int>, "Template parameter must be an integer type");
    switch (standard) {
        case TuningStandard::Baroque:
            return 415.0f;
            break;
        case TuningStandard::Classical:
            return 430.0f;
            break;
        case TuningStandard::Verdi:
            return 432.0f;
            break;
        case TuningStandard::Modern:
            return 440.0f;
            break;
        case TuningStandard::High:
            return 444.0f;
            break;
    }

    return 440.0f; // fallback to modern standard
}

template <typename Int>
inline static float toFrequency (Int standard)
{
    return tuningStandardFrequency (static_cast<int> (standard));
}

template <typename Int>
inline static const char* toString (Int standard)
{
    return tuningStandardString (static_cast<int> (standard));
}

struct Tuning {
    std::string name;
    float sourceFrequency { 440.f };
    float targetFrequency { 432.f };

    static const std::vector<Tuning>& factory() noexcept
    {
        static std::vector<Tuning> _factory;
        if (_factory.empty()) {
            _factory = {
                { "Modern to Verdi (440/432 Hz)", toFrequency (TuningStandard::Modern), toFrequency (TuningStandard::Verdi) },
                { "Modern to Classical (440/430 Hz)", toFrequency (TuningStandard::Modern), toFrequency (TuningStandard::Classical) },
                { "Modern to Baroque (440/415 Hz)", toFrequency (TuningStandard::Modern), toFrequency (TuningStandard::Baroque) },
                { "High to Modern (444/440 Hz)", toFrequency (TuningStandard::High), toFrequency (TuningStandard::Modern) },
                { "High to Verdi (444/432 Hz)", toFrequency (TuningStandard::High), toFrequency (TuningStandard::Verdi) },
                { "Classical to Modern (430/440 Hz)", toFrequency (TuningStandard::Classical), toFrequency (TuningStandard::Modern) },
                { "Classical to Verdi (430/432 Hz)", toFrequency (TuningStandard::Classical), toFrequency (TuningStandard::Verdi) },
                { "Baroque to Modern (415/440 Hz)", toFrequency (TuningStandard::Baroque), toFrequency (TuningStandard::Modern) },
                { "Baroque to Verdi (415/432 Hz)", toFrequency (TuningStandard::Baroque), toFrequency (TuningStandard::Verdi) },
                { "Verdi to Modern (432/440 Hz)", toFrequency (TuningStandard::Verdi), toFrequency (TuningStandard::Modern) }
            };
        }
        return _factory;
    }
};

} // namespace retuner
