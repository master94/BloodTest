#include "TestAnalysis_jni.h"

#include <map>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <analysis.h>

#define LOG_TAG "TestAnalysis_native"
#define LOGD(...) ( (void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__) )


jobject buildTestResult(JNIEnv *jenv, const TestAnalysis::TestInfo &testInfo)
{
    const jclass cls = jenv->FindClass("org/master/bloodtestapp/TestResult");
    const jmethodID constructor = jenv->GetMethodID(cls, "<init>", "()V");

    const jfieldID qc1field = jenv->GetFieldID(cls, "qc1Variation", "D");
    const jfieldID qc2field = jenv->GetFieldID(cls, "qc2Variation", "D");
    const jfieldID valuesField = jenv->GetFieldID(cls, "values", "[D");

    jobject testResult = jenv->NewObject(cls, constructor);
    jenv->SetDoubleField(testResult, qc1field, testInfo.qc1Variation);
    jenv->SetDoubleField(testResult, qc2field, testInfo.qc2Variation);

    jobject valuesObject = jenv->GetObjectField(testResult, valuesField);
    jdoubleArray *valuesArray = reinterpret_cast<jdoubleArray *>(&valuesObject);
    jdouble *values = jenv->GetDoubleArrayElements(*valuesArray, nullptr);
    const jint len = jenv->GetArrayLength(*valuesArray);

    assert(len == 9);

    for (int i = 0; i < len; ++i) {
        const auto sampleType = static_cast<TestAnalysis::SampleType>(i);
        values[i] = testInfo.conc.at(sampleType);
    }

    jenv->SetDoubleArrayRegion(*valuesArray, 0, len, values);
    jenv->ReleaseDoubleArrayElements(*valuesArray, values, 0);

    return testResult;
}

JNIEXPORT jobject JNICALL Java_org_master_bloodtestapp_BloodTestImageAnalyzer_analyzeImage
(JNIEnv *jenv, jclass, jlong imagePtr)
{
	LOGD("Enter native analysis");
    TestAnalysis::TestInfo testInfo;

    try {
		const cv::Mat &image = *reinterpret_cast<cv::Mat *>(imagePtr);
        testInfo = TestAnalysis::detectConcentrations(image);
	}
	catch (cv::Exception &e) {
		LOGD("nativeCreateObject caught cv::Exception: %s", e.what());
		jclass je = jenv->FindClass("org/opencv/core/CvException");
		jenv->ThrowNew(je, e.what());
	}
	catch (...) {
		LOGD("Unknown exception");
		jclass je = jenv->FindClass("java/lang/Exception");
		jenv->ThrowNew(je, "Unknown exception in JNI code");
	}

	LOGD("Exit image analysis");

    return buildTestResult(jenv, testInfo);
}
