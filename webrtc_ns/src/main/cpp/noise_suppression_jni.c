//
// Created by 汪洋 on 2018/12/7.
//

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <android/log.h>

//ref:https://github.com/mackron/dr_libs/blob/master/dr_wav.h
#define DR_WAV_IMPLEMENTATION
#define JNIEXPORT __attribute__ ((visibility ("default")))

#include "timing.h"
#include "dr_wav.h"
#include "noise_suppression.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

#define LOGV(...)   __android_log_print((int)ANDROID_LOG_INFO, "WEBTRC_NS", __VA_ARGS__)

//写wav文件
static void wavWrite_int16(const char *filename, int16_t *buffer, size_t sampleRate, size_t totalSampleCount) {
    drwav_data_format format = {};
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 1;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, totalSampleCount, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != totalSampleCount) {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }
}

//读取wav文件
static int16_t *wavRead_int16(const char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount) {
    unsigned int channels;
    int16_t *buffer = drwav_open_and_read_file_s16(filename, &channels, sampleRate, totalSampleCount);
    if (buffer == nullptr) {
        printf("ERROR.");
    }
    return buffer;
}

enum nsLevel {
    kLow = 0,
    kModerate,
    kHigh,
    kVeryHigh
};

// 降噪
static int nsProcessWithBuffer(int16_t *buffer, uint32_t sampleRate, int samplesCount, enum nsLevel level) {
    if (buffer == nullptr) return -1;
    if (samplesCount == 0) return -1;
    size_t samples = MIN(160, sampleRate / 100);
    if (samples == 0) return -1;
    uint32_t num_bands = 1;
    int16_t *input = buffer;
    size_t nTotal = (samplesCount / samples);
    NsHandle *nsHandle = WebRtcNs_Create();
    int status = WebRtcNs_Init(nsHandle, sampleRate);
    if (status != 0) {
        printf("WebRtcNs_Init fail\n");
        return -1;
    }
    status = WebRtcNs_set_policy(nsHandle, level);
    if (status != 0) {
        printf("WebRtcNs_set_policy fail\n");
        return -1;
    }
    for (int i = 0; i < nTotal; i++) {
        int16_t *nsIn[1] = {input};   //ns input[band][data]
        int16_t *nsOut[1] = {input};  //ns output[band][data]
        WebRtcNs_Analyze(nsHandle, nsIn[0]);
        WebRtcNs_Process(nsHandle, (const int16_t *const *) nsIn, num_bands, nsOut);
        input += samples;
    }
    WebRtcNs_Free(nsHandle);

    return 1;
}

// 降噪
static void nsProcessWithFile(const char *in_file, const char *out_file, enum nsLevel level) {
    //音频采样率
    uint32_t sampleRate = 0;
    //总音频采样数
    uint64_t inSampleCount = 0;
    int16_t *inBuffer = wavRead_int16(in_file, &sampleRate, &inSampleCount);

    //如果加载成功
    if (inBuffer != nullptr) {
        double startTime = now();
        nsProcessWithBuffer(inBuffer, sampleRate, inSampleCount, level);
        double time_interval = calcElapsed(startTime, now());
        printf("time interval: %d ms\n ", (int) (time_interval * 1000));
        wavWrite_int16(out_file, inBuffer, sampleRate, inSampleCount);
        free(inBuffer);
    }
}

JNIEXPORT void Java_com_aiyaapp_webrtc_ns_WebRTCNoiseSuppression_process(JNIEnv *env, jobject thiz, jstring jinputFile, jstring joutputFile, jint level) {

    const char *inputFile = (*env)->GetStringUTFChars(env, jinputFile, 0);
    const char *outputFile = (*env)->GetStringUTFChars(env, joutputFile, 0);

    nsProcessWithFile(inputFile, outputFile, (enum nsLevel) level);
}