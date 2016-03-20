#ifndef _Included_org_master_bloodtestapp_BloodTestImageAnalyzer
#define _Included_org_master_bloodtestapp_BloodTestImageAnalyzer

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jobject JNICALL Java_org_master_bloodtestapp_BloodTestImageAnalyzer_analyzeImage
(JNIEnv *, jclass, jlong image);

#ifdef __cplusplus
}
#endif

#endif
