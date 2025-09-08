#include <jni.h>
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"VoiceChanger",__VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
Java_com_shepamike_voicechanger_MainActivity_nativeStart(JNIEnv*, jobject){
    LOGI("nativeStart called");
    // TODO: 音声処理を追加
}
extern "C" JNIEXPORT void JNICALL
Java_com_shepamike_voicechanger_MainActivity_nativeStop(JNIEnv*, jobject){
    LOGI("nativeStop called");
}
