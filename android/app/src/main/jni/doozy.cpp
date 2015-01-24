#include <jni.h>
#include <android/log.h>
#include <memory>

#include "../../../../../common/doozydeviceinterface.h"
#include "../../../../../common/doozydevicefactory.h"

#define  LOG_TAG    "Doozydroid"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static std::unique_ptr<doozy::IDevice> g_deviceInstance;

bool Java_com_divabo_doozy_run_server(JNIEnv* env, jobject thiz)
{
    try
    {
        LOGI("Start renderer");
        g_deviceInstance = doozy::DeviceFactory::createDevice("renderer", "");
        g_deviceInstance->start();
        LOGI("Finished");
        return true;
    }
    catch (std::exception& e)
    {
        LOGE("Exception occurred: %s", e.what());
        return false;
    }
}
