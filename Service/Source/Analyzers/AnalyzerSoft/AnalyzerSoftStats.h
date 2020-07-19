// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftStats final {
  private :
    static constexpr float   FrequencyRanges[]   = { -INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY };
    static constexpr uint8_t FrequencyRangesLen  = (sizeof(FrequencyRanges) / sizeof(float)) - 1;
    static constexpr float   VoltageRanges[]     = { -INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY };
    static constexpr uint8_t VoltageRangesLen    = (sizeof(VoltageRanges) / sizeof(float)) - 1;
    static constexpr float   THDRanges[]         = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY };
    static constexpr uint8_t THDRangesLen        = (sizeof(THDRanges) / sizeof(float)) - 1;
    static constexpr float   UnbalanceRanges[]   = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.25, 2.50, 2.75, 3.0, 3.25, 3.50, 3.75, 4.0, 4.5, 5.0, INFINITY};
    static constexpr uint8_t UnbalanceRangesLen  = (sizeof(UnbalanceRanges) / sizeof(float)) - 1;
    static constexpr float   HarmonicsAllowed[]  = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
    static constexpr uint8_t HarmonicsAllowedLen = (sizeof(HarmonicsAllowed) / sizeof(float));
  public :
    AnalyzerSoftStats();
    ~AnalyzerSoftStats();

// ---------------------------------------------------------------------------------------------------------------------