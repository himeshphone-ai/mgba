#include <jni.h>
#include <string.h>
#include <mgba/internal/gba/gba.h>

static JavaVM* g_vm = NULL;
static jobject g_activityObj = NULL;

// Save Java VM instance on library load
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_vm = vm;
        return JNI_VERSION_1_6;
        }

        // Store global reference to Java Activity/View
        JNIEXPORT void JNICALL
        Java_com_mgba_GbaActivity_initNativeBridge(JNIEnv *env, jobject thiz) {
            if (g_activityObj) {
                    (*env)->DeleteGlobalRef(env, g_activityObj);
                        }
                            g_activityObj = (*env)->NewGlobalRef(env, thiz);
                            }

                            // 1. Called by C hook in gba.c to trigger Android Gboard dialog
                            void Android_ShowKeyboard(void) {
                                if (!g_vm || !g_activityObj) return;

                                    JNIEnv* env;
                                        (*g_vm)->AttachCurrentThread(g_vm, &env, NULL);

                                            jclass clazz = (*env)->GetObjectClass(env, g_activityObj);
                                                jmethodID methodId = (*env)->GetMethodID(env, clazz, "showNativeKeyboard", "()V");
                                                    if (methodId) {
                                                            (*env)->CallVoidMethod(env, g_activityObj, methodId);
                                                                }
                                                                }

                                                                // 2. Called by Java when user finishes typing in Gboard
                                                                JNIEXPORT void JNICALL
                                                                Java_com_mgba_GbaActivity_nativeSendTextToGba(JNIEnv *env, jobject thiz, jstring text, jlong gbaPtr) {
                                                                    struct GBA* gba = (struct GBA*)gbaPtr;
                                                                        if (!gba || !gba->memory.wram) return;

                                                                            const char *nativeString = (*env)->GetStringUTFChars(env, text, 0);
                                                                                uint8_t* ewram = (uint8_t*)gba->memory.wram;

                                                                                    // Write text to 0x0203CE04 (offset 0x3CE04)
                                                                                        size_t len = strlen(nativeString);
                                                                                            for (size_t i = 0; i < len && i < 15; i++) {
                                                                                                    ewram[0x3CE04 + i] = (uint8_t)nativeString[i];
                                                                                                        }
                                                                                                ewram[0x3CE04 + (len < 15 ? len : 15)] = '\0'; // Null-terminate

                                                                                                                // Set status to 2 (Signals complete back to pokeemerald ROM)
                                                                                                                    ewram[0x3CE00] = 2;

                                                                                                                        (*env)->ReleaseStringUTFChars(env, text, nativeString);
                                                                                                                        }
