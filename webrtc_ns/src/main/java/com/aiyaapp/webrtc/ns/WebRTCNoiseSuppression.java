package com.aiyaapp.webrtc.ns;

/**
 * Created by 汪洋 on 2018/12/7.
 * Copyright © 2018年 深圳市云歌人工智能技术有限公司. All rights reserved.
 */
public class WebRTCNoiseSuppression {

    public native static void process(String inputFile, String outputFile, int level);

    static {
        System.loadLibrary("webrtc_ns");
    }
}
