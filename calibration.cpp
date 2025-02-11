#include "calibration.h"

// Updated Calibration Table (With Previously Applied Factors Considered)
CalibrationPoint calibrationTable[] = {
    {1, 0.483234},
    {5, 0.891862},
    {10, 0.966865},
    {20, 0.982273},
    {50, 0.996319},
    {70, 0.999675},
    {100, 1.000000}
};

const int calibrationTableSize = sizeof(calibrationTable) / sizeof(calibrationTable[0]);

// Function to find interpolated correction factor
float getInterpolatedCorrection(float targetLength) {
    if (targetLength <= calibrationTable[0].expectedLength) {
        return calibrationTable[0].correctionFactor;
    }
    if (targetLength >= calibrationTable[calibrationTableSize - 1].expectedLength) {
        return calibrationTable[calibrationTableSize - 1].correctionFactor;
    }

    // Find two points for interpolation
    for (int i = 0; i < calibrationTableSize - 1; i++) {
        if (targetLength >= calibrationTable[i].expectedLength && targetLength <= calibrationTable[i + 1].expectedLength) {
            float x0 = calibrationTable[i].expectedLength;
            float y0 = calibrationTable[i].correctionFactor;
            float x1 = calibrationTable[i + 1].expectedLength;
            float y1 = calibrationTable[i + 1].correctionFactor;

            // Linear interpolation formula
            return y0 + ((targetLength - x0) * (y1 - y0)) / (x1 - x0);
        }
    }
    return 1.0; // Default correction factor if not found
}

// Apply correction factor
float getAdjustedCutDistance(float targetLength) {
    float correctionFactor = getInterpolatedCorrection(targetLength);
    return targetLength * correctionFactor;
}

