#ifndef CALIBRATION_H
#define CALIBRATION_H

// Struct for calibration points
struct CalibrationPoint {
    float expectedLength;
    float correctionFactor;
};

// Function prototypes
float getInterpolatedCorrection(float targetLength);
float getAdjustedCutDistance(float targetLength);

#endif // CALIBRATION_H
