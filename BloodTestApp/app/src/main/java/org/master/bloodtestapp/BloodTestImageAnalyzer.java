package org.master.bloodtestapp;

/**
 * Created by master on 3/17/16.
 */

import android.util.Log;

import org.opencv.core.Mat;

public class BloodTestImageAnalyzer {
    public BloodTestImageAnalyzer() {
    }

    public TestResult analyze(Mat image) {
        return analyzeImage(image.getNativeObjAddr());
    }

    private native TestResult analyzeImage(long imageNativeId);
}
