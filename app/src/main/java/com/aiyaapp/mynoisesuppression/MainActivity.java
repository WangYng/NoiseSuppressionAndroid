package com.aiyaapp.mynoisesuppression;

import android.content.Intent;
import android.net.Uri;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.aiyaapp.webrtc.ns.WebRTCNoiseSuppression;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    File testWavFile = null;
    File outputFile = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        testWavFile =  new File(getExternalCacheDir(), "test.wav");
        outputFile = new File(getExternalCacheDir(), "soundtouch-output.wav");

        // 把asserts/webrtc_ns_test.wav 文件转存到 cache/test.wav
        try {
            InputStream is = getAssets().open("webrtc_ns_test.wav");
            OutputStream os = new FileOutputStream(testWavFile);

            byte[] buffer = new byte[1024];
            int length;
            while ((length = is.read(buffer)) > 0) {
                os.write(buffer, 0, length);
            }

            is.close();
            os.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
        {
            Button buttonProcess = findViewById(R.id.ns_button1);
            buttonProcess.setOnClickListener(this);
        }
        {
            Button buttonProcess = findViewById(R.id.ns_button2);
            buttonProcess.setOnClickListener(this);
        }
        {
            Button buttonProcess = findViewById(R.id.ns_button3);
            buttonProcess.setOnClickListener(this);
        }
        {
            Button buttonProcess = findViewById(R.id.ns_button4);
            buttonProcess.setOnClickListener(this);
        }
        Button origin = findViewById(R.id.origin_button);
        origin.setOnClickListener(this);
    }

    @Override
    public void onClick(final View v) {
        if (v.getId() == R.id.origin_button) {
            playWavFile(testWavFile.getAbsolutePath());

        }else if (v.getId() == R.id.ns_button1) {
            noisePressionProcess(0);

        }else if (v.getId() == R.id.ns_button2) {
            noisePressionProcess(1);

        }else if (v.getId() == R.id.ns_button3) {
            noisePressionProcess(2);

        }else if (v.getId() == R.id.ns_button4) {
            noisePressionProcess(3);
        }
    }

    protected void noisePressionProcess(final int level) {
        final long startTime = SystemClock.elapsedRealtime();

        new Thread(){
            @Override
            public void run() {
                WebRTCNoiseSuppression.process(testWavFile.getAbsolutePath(), outputFile.getAbsolutePath(), level);

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {

                        long endTime = SystemClock.elapsedRealtime();
                        TextView textView = findViewById(R.id.ns_duration_textview);
                        textView.setText("降噪耗时: " + (endTime - startTime) + "ms");

                        playWavFile(outputFile.getAbsolutePath());
                    }
                });
            }
        }.start();
    }

    protected void playWavFile(String fileName)
    {
        File file2play = new File(fileName);
        Intent i = new Intent();
        i.setAction(android.content.Intent.ACTION_VIEW);
        i.setDataAndType(Uri.fromFile(file2play), "audio/wav");
        startActivity(i);
    }
}
