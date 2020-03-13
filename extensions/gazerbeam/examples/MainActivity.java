package org.iocafe.blinky;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.hardware.camera2.*;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Process;
// import android.system.OsConstants;
// import android.system.Os;


public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // doBlink();

        /* final Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                turnOnLight();
                //Do something after 100ms
            }
        }, 10);
        */

        CameraManager objCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        // new DownloadImageTask().execute(objCameraManager);

        new DownloadImageTask().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }


    private class DownloadImageTask extends AsyncTask<CameraManager, Integer, Long> {
        /** The system calls this to perform work in a worker thread and
         * delivers it the parameters given to AsyncTask.execute() */
        protected Long doInBackground(CameraManager... objCameraManagers) {

            // CameraManager objCameraManager = objCameraManagers[0];
            CameraManager objCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);

            try {
//            objCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
                mCameraId = objCameraManager.getCameraIdList()[0];
                // timer = new Timer();
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

        // public CameraManager objCameraManager;
        private String mCameraId;
        private boolean uu;


        /** The system calls this to perform work in the UI thread and delivers
         * the result from doInBackground() */
        //protected void onPostExecute(Bitmap result) {
            // mImageView.setImageBitmap(result);
        //}
    }


    /* public void doBlink() {
        try {
            objCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
            mCameraId = objCameraManager.getCameraIdList()[0];
            timer = new Timer();

        } catch (Exception e) {
            e.printStackTrace();
        }

        timer.schedule(new TimerTask() {
            public void run() {
                try {
                    objCameraManager.setTorchMode(mCameraId, uu);
                    uu = !uu;
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }, 0, 5);
    } */


    /* public void turnOnLight() {
        try {
            objCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
            mCameraId = objCameraManager.getCameraIdList()[0];
            objCameraManager.setTorchMode(mCameraId, uu);
            uu = !uu;
        } catch (Exception e) {
            e.printStackTrace();
        }

        final Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                turnOnLight();
                //Do something after 100ms
            }
        }, 10);

    } */

    // private CameraManager objCameraManager;
    // private String mCameraId;
    // private Timer timer;
    // private boolean uu;
}

