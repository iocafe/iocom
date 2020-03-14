package com.ioface.blinky;

import android.os.Bundle;

import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import android.widget.ToggleButton;
import android.widget.CompoundButton;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.view.Menu;
import android.view.MenuItem;

import android.hardware.camera2.*;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.AsyncTask;
import android.os.Process;


public class MainActivity extends AppCompatActivity
{
    protected sendData
            m_sender;

    protected TextInputLayout
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

    protected ToggleButton
            m_blink_button;

    protected String
            m_wifi_network,
            m_wifi_password,
            io_network_name,
            m_device_number,
            m_connect_ip;

    protected SharedPreferences
            m_pref;

    protected AsyncTask<Void, Void, Boolean>
            m_task;

    protected boolean
            m_task_running,
            m_started;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        m_task_running = false;
        m_started = false;

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

        m_blink_button = findViewById(R.id.blink_button);
        m_blink_button.setChecked(false);
        m_blink_button.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    if (m_started) {
                        startLED();
                    }
                    else {
                        m_blink_button.setChecked(false);
                    }
                } else {
                    stopLED();
                }
            }
        });

        m_pref = getApplicationContext().getSharedPreferences("BlinkyPref", Context.MODE_PRIVATE);
    }

    @Override
    protected void onStart()
    {
        loadState();
        setUiState();
        super.onStart();
        m_started = true;
    }

    @Override
    protected void onStop()
    {
        getUiState();
        saveState();
        super.onStop();
        m_started = false;
    }

    protected void startLED()
    {
        stopLED();

        enableUiState(false);
        getUiState();
        int data[] = makeMessageData();

        sendData sender = new sendData();
        sender.setByteData(data);
        m_task = sender.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        m_task_running = true;
    }

    protected void stopLED()
    {
        if (m_task_running) {
            m_task.cancel(true);
            m_task_running = false;
            enableUiState(true);
        }
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

    protected void enableUiState(boolean enable) {
        m_wifi_network_edit.setEnabled(enable);
        m_wifi_password_edit.setEnabled(enable);
        io_network_name_edit.setEnabled(enable);
        m_device_number_edit.setEnabled(enable);
        m_connect_ip_edit.setEnabled(enable);
    }

    protected void saveState()
    {
        Editor editor = m_pref.edit();
        editor.putString("wifi_network", m_wifi_network);
        editor.putString("wifi_password", m_wifi_password);
        editor.putString("io_network_name", io_network_name);
        editor.putString("device_number", m_device_number);
        editor.putString("connect_ip", m_connect_ip);
        editor.commit();
    }

    protected void loadState()
    {
        m_wifi_network = m_pref.getString("wifi_network", "");
        m_wifi_password = m_pref.getString("wifi_password", "");
        io_network_name = m_pref.getString("io_network_name", "");
        m_device_number = m_pref.getString("device_number", "");
        m_connect_ip = m_pref.getString("connect_ip", "");
    }

    protected int[] makeMessageData()
    {
        int wn[] = convertString(m_wifi_network, 1);
        int wp[] = convertString(m_wifi_password, 2);
        int nn[] = convertString(io_network_name, 3);
        int dn[] = convertString(m_device_number, 4);
        int ip[] = convertString(m_connect_ip, 5);

        int n = wn.length + wp.length;
        int data[] = new int [n];
        int pos = 0;

        int item[];

        item = wn;
        if (item.length > 1) {
            appendItem(data, pos, item);
            pos += item.length;
        }

        item = wp;
        if (item.length > 1) {
            appendItem(data, pos, item);
            pos += item.length;
        }

        item = nn;
        if (item.length > 1) {
            appendItem(data, pos, item);
            pos += item.length;
        }

        item = dn;
        if (item.length > 1) {
            appendItem(data, pos, item);
            pos += item.length;
        }

        item = ip;
        if (item.length > 1) {
            appendItem(data, pos, item);
            pos += item.length;
        }
        return data;
    }

    // Maximum value x length 31 characters
    protected int[] convertString(String x, int id)
    {
        byte[] b = x.getBytes();

        int n = b.length + 1;
        int item[] = new int[n];

        item[0] = (id << 5) | b.length;
        for (int i = 0; i < b.length; i++)
        {
            item[i+1] = converUnsignedValue(b[i]);
        }

        return item;
    }

    protected int converUnsignedValue(Byte x) {
        // auto cast to int
        return x & 0xFF;
    }

    protected void appendItem(int data[], int pos, int item[])
    {
        for (int i = 0; i < item.length; i++)
        {
            data[i + pos] = item[i];
        }
    }

    private class sendData extends AsyncTask<Void, Void, Boolean> {

        protected int m_short_pulse_ms;
        protected int m_long_pulse_ms;
        protected boolean m_led_on;

        public CameraManager m_camera_manager;
        private String m_camera_id;
        int m_data[];

        /**
         * The system calls this to perform work in a worker thread and
         * delivers it the parameters given to AsyncTask.execute()
         */
        @Override
        protected Boolean doInBackground(Void... params) {

            m_short_pulse_ms = 30;
            m_long_pulse_ms = 100;
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

            while (!isCancelled()) {
                // Send 10 zeroes followed by 1
                for (int i = 0; i < 10; i++) {
                    toggleLed(m_short_pulse_ms);
                }
                toggleLed(m_long_pulse_ms);

                // Send actual data bytes, each followed by 0 and 1
                for (int i = 0; i < len && !isCancelled(); i++) {
                    int v = m_data[i];
                    for (int j = 0; j < 8 && !isCancelled(); j++) {
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

            if (m_led_on)
            {
                toggleLed(0);
            }
            return true;
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

        public void setByteData(int data[]) {
            m_data = data;
        }
    }
}
