package com.ioface.blinky;

import android.os.Bundle;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.view.View;
import android.view.Menu;
import android.view.MenuItem;

import android.hardware.camera2.*;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Process;


public class MainActivity extends AppCompatActivity {

    protected sendData sender;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        byte b[];
        b = new byte [4];
        b[0] = 100;
        b[1] = 5;
        b[2] = 55;
        b[3] = 15;

        sendData sender = new sendData();
        sender.setByteData(b);
        sender.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private class sendData extends AsyncTask<Void, Void, Boolean> {
        /**
         * The system calls this to perform work in a worker thread and
         * delivers it the parameters given to AsyncTask.execute()
         */
        @Override
        protected Boolean doInBackground(Void... params) {

            m_short_pulse_ms = 300;
            m_long_pulse_ms = 1000;
            m_led_on = false;

            // CameraManager objCameraManager = objCameraManagers[0];
            m_camera_manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);

            try {
                m_camera_id = m_camera_manager.getCameraIdList()[0];
                Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);

            } catch (Exception e) {
                e.printStackTrace();
            }

            int len = m_data.length;

            while (true) {
                // Send 10 zeroes followed by 1
                for (int i = 0; i < 10; i++) {
                    toggleLed(m_short_pulse_ms);
                }
                toggleLed(m_long_pulse_ms);

                // Send actual data bytes, each followed by 0 and 1
                for (int i = 0; i < len; i++) {
                    int v = m_data[i];
                    for (int j = 0; j < 8; j++) {
                        if ((v & 1) == 1)
                        {
                            toggleLed(m_long_pulse_ms);
                        }
                        else
                        {
                            toggleLed(m_short_pulse_ms);
                        }
                        v >>= 1;
                    }
                    toggleLed(m_short_pulse_ms);
                    toggleLed(m_long_pulse_ms);
                }
            }
            // return true;
        }

        protected void toggleLed(int pulse_ms) {
            m_led_on = !m_led_on;

            try {
                m_camera_manager.setTorchMode(m_camera_id, m_led_on);
                Thread.sleep(pulse_ms, 0);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void setByteData(byte data[]) {
            m_data = data;
        }

        protected int m_short_pulse_ms;
        protected int m_long_pulse_ms;
        protected boolean m_led_on;

        public CameraManager m_camera_manager;
        private String m_camera_id;
        byte m_data[];


        /** The system calls this to perform work in the UI thread and delivers
         * the result from doInBackground() */
        //protected void onPostExecute(Bitmap result) {
        // mImageView.setImageBitmap(result);
        //}
    }
}
