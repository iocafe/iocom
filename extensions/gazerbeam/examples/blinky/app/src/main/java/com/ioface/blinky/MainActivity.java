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
        new sendData().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
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

    private class sendData extends AsyncTask<String, Integer, Long> {
        /** The system calls this to perform work in a worker thread and
         * delivers it the parameters given to AsyncTask.execute() */
        protected Long doInBackground(String... data) {

            // CameraManager objCameraManager = objCameraManagers[0];
            objCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);

            try {
                mCameraId = objCameraManager.getCameraIdList()[0];
                Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);

            } catch (Exception e) {
                e.printStackTrace();
            }

            while (true) {
                try {
                    objCameraManager.setTorchMode(mCameraId, uu);
                    uu = !uu;
                    Thread.sleep(10, 0);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        public CameraManager objCameraManager;
        private String mCameraId;
        private boolean uu;


        /** The system calls this to perform work in the UI thread and delivers
         * the result from doInBackground() */
        //protected void onPostExecute(Bitmap result) {
        // mImageView.setImageBitmap(result);
        //}
    }
}
