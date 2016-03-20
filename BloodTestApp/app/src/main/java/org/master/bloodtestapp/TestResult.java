package org.master.bloodtestapp;

import java.util.HashMap;

/**
 * Created by master on 3/20/16.
 */
public class TestResult {
    public final double[] values = new double[9];
    public double qc1Variation;
    public double qc2Variation;

    public TestResult() {
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();

        for (int i = 0; i < values.length; ++i) {
            builder.append(String.format("Value: %f\n", values[i]));
        }
        builder.append(String.format("QC1 VAR: %f\n", qc1Variation));
        builder.append(String.format("QC2 VAR: %f\n", qc2Variation));
        return builder.toString();
    }
}
