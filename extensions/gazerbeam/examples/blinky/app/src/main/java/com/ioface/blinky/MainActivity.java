package com.ioface.blinky;

import android.os.Bundle;

import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.view.Menu;
import android.view.MenuItem;

import android.hardware.camera2.*;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Process;


public class MainActivity extends AppCompatActivity
{
    protected sendData
            m_sender;

    TextInputLayout
            m_wifi_network_layout,
            m_wifi_password_layout,
            io_network_name_layout,
            m_device_number_layout,
            m_connect_ip_layout;

    protected TextInputEditText
            m_wifi_network_edit,
            m_wifi_password_edit,
            io_network_name_edit,
            m_device_number_edit,
            m_connect_ip_edit;

    String
            m_wifi_network,
            m_wifi_password,
            io_network_name,
            m_device_number,
            m_connect_ip;


    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        m_wifi_network_layout = findViewById(R.id.wifi_network_layout);
        m_wifi_password_layout = findViewById(R.id.wifi_password_layout);
        io_network_name_layout = findViewById(R.id.io_network_name_layout);
        m_device_number_layout = findViewById(R.id.device_number_layout);
        m_connect_ip_layout = findViewById(R.id.connect_ip_layout);

        m_wifi_network_edit = findViewById(R.id.wifi_network_edit);
        m_wifi_password_edit = findViewById(R.id.wifi_password_edit);
        io_network_name_edit = findViewById(R.id.io_network_name_edit);
        m_device_number_edit = findViewById(R.id.device_number_edit);
        m_connect_ip_edit = findViewById(R.id.connect_ip_edit);

        // m_wifi_network_edit.setText("Nasse");

        getUiState();


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
    public boolean onCreateOptionsMenu(Menu menu)
    {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
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

    protected void getUiState()
    {
        m_wifi_network = m_wifi_network_edit.getText().toString();
        m_wifi_password = m_wifi_password_edit.getText().toString();
        io_network_name = io_network_name_edit.getText().toString();
        m_device_number = m_device_number_edit.getText().toString();
        m_connect_ip = m_connect_ip_edit.getText().toString();
    }

    protected void setUiState()
    {
        m_wifi_network_edit.setText(m_wifi_network);
        m_wifi_password_edit.setText(m_wifi_password);
        io_network_name_edit.setText(io_network_name);
        m_device_number_edit.setText(m_device_number);
        m_connect_ip_edit.setText(m_connect_ip);
    }

    protected Bundle saveState()
    {
        Bundle b = new Bundle();

        b.putString("wifi_network", m_wifi_network);
        b.putString("wifi_password", m_wifi_password);
        b.putString("io_network_name", io_network_name);
        b.putString("device_number", m_device_number);
        b.putString("connect_ip", m_connect_ip);
        return b;
    }

    protected void loadState(Bundle b) {
        m_wifi_network = b.getString("wifi_network");
        m_wifi_password = b.getString("wifi_password");
        io_network_name = b.getString("io_network_name");
        m_device_number = b.getString("device_number");
        m_connect_ip = b.getString("connect_ip");
    }

    // Storing bundle object into intent
//                    in.putExtras(b);

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
