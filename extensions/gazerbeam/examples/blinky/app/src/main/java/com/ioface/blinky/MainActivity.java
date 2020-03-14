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
        /** The system calls this to perform work in a worker thread and
         * delivers it the parameters given to AsyncTask.execute() */
        @Override
        protected Boolean doInBackground(Void... params) {

            // CameraManager objCameraManager = objCameraManagers[0];
            mCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);

            try {
                mCameraId = mCameraManager.getCameraIdList()[0];
                Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);

            } catch (Exception e) {
                e.printStackTrace();
            }

            int pos = 0;
            int len = mData.length;
            boolean led_on = false;

            while (true) {
                try {
                    led_on = !led_on;
                    mCameraManager.setTorchMode(mCameraId, led_on);

                    Thread.sleep(mData[pos], 0);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                pos = pos + 1;
                if (pos >= len) pos = 0;
            }
            // return true;
        }

        public void setByteData(byte data[]) {
            mData = data;
        }

        public CameraManager mCameraManager;
        private String mCameraId;
        byte mData[];


        /** The system calls this to perform work in the UI thread and delivers
         * the result from doInBackground() */
        //protected void onPostExecute(Bitmap result) {
        // mImageView.setImageBitmap(result);
        //}
    }
}
