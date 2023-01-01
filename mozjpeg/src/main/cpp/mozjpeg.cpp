#include <jni.h>
#include <string>
#include "turbojpeg.h"
#include <android/bitmap.h>
#include <vector>
#include <arm_fp16.h>
#include <cfloat>
#include "libyuv/libyuv.h"

jint throwEncodeException(JNIEnv *env, const char *msg) {
    jclass exClass;
    exClass = env->FindClass("com/github/awxkee/mozjpeg/EncodeJpegException");
    return env->ThrowNew(exClass, msg);
}

jint throwPixelsException(JNIEnv *env) {
    jclass exClass;
    exClass = env->FindClass("com/github/awxkee/mozjpeg/GetPixelsException");
    return env->ThrowNew(exClass, "");
}

jint throwInvalidPixelsFormat(JNIEnv *env) {
    jclass exClass;
    exClass = env->FindClass("com/github/awxkee/mozjpeg/UnsupportedImageFormatException");
    return env->ThrowNew(exClass, "");
}

jint throwHardwareBitmapException(JNIEnv *env) {
    jclass exClass;
    exClass = env->FindClass("com/github/awxkee/mozjpeg/HardwareBitmapIsNotImplementedException");
    return env->ThrowNew(exClass, "");
}

extern "C"
JNIEXPORT jbyteArray

JNICALL
Java_com_github_awxkee_mozjpeg_Mozjpeg_compressImpl(JNIEnv *env, jobject thiz, jobject bitmap,
                                                    jint mQuality) {
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
        throwPixelsException(env);
        return static_cast<jbyteArray>(nullptr);
    }

    if (info.flags & ANDROID_BITMAP_FLAGS_IS_HARDWARE) {
        throwHardwareBitmapException(env);
        return static_cast<jbyteArray>(nullptr);
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 &&
        info.format != ANDROID_BITMAP_FORMAT_RGB_565 &&
        info.format != ANDROID_BITMAP_FORMAT_RGBA_F16 &&
        info.format != ANDROID_BITMAP_FORMAT_RGBA_1010102) {
        throwInvalidPixelsFormat(env);
        return static_cast<jbyteArray>(nullptr);
    }

    void *addr;
    if (AndroidBitmap_lockPixels(env, bitmap, &addr) != 0) {
        throwPixelsException(env);
        return static_cast<jbyteArray>(nullptr);
    }

    std::shared_ptr<char> dstARGB(
            static_cast<char *>(malloc(info.width * 4 * info.height)),
            [](char *f) { free(f); });
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        memcpy(dstARGB.get(), addr, info.stride * info.height);
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGBA_1010102) {
        libyuv::AR30ToABGR(static_cast<const uint8_t *>(addr), (int) info.stride,
                           reinterpret_cast<uint8_t *>(dstARGB.get()),
                           (int) info.width * 4, (int) info.width,
                           (int) info.height);
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        libyuv::RGB565ToARGB(static_cast<const uint8_t *>(addr), (int) info.stride,
                             reinterpret_cast<uint8_t *>(dstARGB.get()),
                             (int) info.width * 4, (int) info.width,
                             (int) info.height);
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGBA_F16) {
        auto *srcData = static_cast<float16_t *>(addr);
        char tmpR;
        char tmpG;
        char tmpB;
        char tmpA;
        auto *data32Ptr = reinterpret_cast<uint32_t *>(dstARGB.get());
        const float maxColors = (float) pow(2.0, 8) - 1;
        for (int i = 0, k = 0; i < std::min(info.stride * info.height,
                                            info.width * info.height * 4); i += 4, k += 1) {
            tmpR = (char) (srcData[i] * maxColors);
            tmpG = (char) (srcData[i + 1] * maxColors);
            tmpB = (char) (srcData[i + 2] * maxColors);
            tmpA = (char) (srcData[i + 3] * maxColors);
            uint32_t color = ((uint32_t) tmpA & 0xff) << 24 | ((uint32_t) tmpB & 0xff) << 16 |
                             ((uint32_t) tmpG & 0xff) << 8 | ((uint32_t) tmpR & 0xff);
            data32Ptr[k] = color;
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);

    std::shared_ptr<void> tjInstance(tjInitCompress(), [](tjhandle h) { tjDestroy(h); });
    unsigned char *jpegBuf = nullptr;
    unsigned long jpegSize;
    int pixelFormat = TJPF_RGBA;
    if (info.format == ANDROID_BITMAP_FORMAT_RGB_565 ||
        info.format == ANDROID_BITMAP_FORMAT_RGBA_1010102) {
        pixelFormat = TJPF_BGRA;
    }
    auto ret = tjCompress2(tjInstance.get(), reinterpret_cast<const unsigned char *>(dstARGB.get()),
                           (int) info.width, 0, (int) info.height, pixelFormat,
                           &jpegBuf, &jpegSize, TJSAMP_420, mQuality, TJFLAG_ACCURATEDCT);
    dstARGB.reset();
    if (ret != 0) {
        tjInstance.reset();
        throwEncodeException(env, tjGetErrorStr2(tjInstance.get()));
        return static_cast<jbyteArray>(nullptr);
    }
    jbyteArray byteArray = env->NewByteArray((jsize) jpegSize);
    char *memBuf = (char *) ((void *) jpegBuf);
    env->SetByteArrayRegion(byteArray, 0, (jint) jpegSize,
                            reinterpret_cast<const jbyte *>(memBuf));
    tjFree(jpegBuf);
    tjInstance.reset();
    return byteArray;
}