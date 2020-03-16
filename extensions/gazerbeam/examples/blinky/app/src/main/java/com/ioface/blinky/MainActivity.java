/**

 @file    MainActivity.c
 @brief   Blinky main activity.
 @author  Pekka Lehtikoski
 @version 1.0
 @date    14.3.2020

 The gazerbeam is small library to allow setting wifi network name (SSID) and password (PSK)
 using LED flash of Android phone, etc. It can also be used for some other simple configuration
 data.

 The Blinky is Android/Java application to blink configuration information, so that the
 microcontroller with phototransistor can receive the information.

 Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
 modified, and distributed under the terms of the project licensing. By continuing to use, modify,
 or distribute this file you indicate that you have read the license and understand and accept
 it fully.

 ****************************************************************************************************
 */
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
// import android.os.AsyncTask;
// import android.os.Process;

import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends AppCompatActivity
{
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

    protected boolean
            m_task_running,
            m_started;

    protected Timer
            m_timer;

    int[] m_crc_table = {
            0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
            0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
            0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
            0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
            0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
            0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
            0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
            0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
            0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
            0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
            0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
            0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
            0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
            0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
            0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
            0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
            0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
            0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
            0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
            0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
            0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
            0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
            0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
            0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
            0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
            0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
            0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
            0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
            0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
            0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
            0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
            0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040};

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

        startFlashing(data);
        m_task_running = true;
    }

    protected void stopLED()
    {
        if (m_task_running) {
            stopFlashing();
            // m_task.cancel(true);
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
        int data[] = new int [n + 2];
        data[0] = 0;
        data[1] = 0;
        int pos = 2;
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

        int crc = os_checksum(data);
        data[0] = crc & 255;
        data[1] = (crc >> 8) & 255;

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

    /* Calculate modbus checksum for the buffer given as an argument.
     */
    protected int os_checksum(
        int buf[])
    {
        int tmp;
        int w = 0xFFFF;

        for (int i = 0; i < buf.length; i++)
        {
            tmp = buf[i] ^ w;
            w >>= 8;
            w ^= m_crc_table[tmp & 255];
        }

        return w;
    }


    protected byte[] makeRecipe(int data[])
    {
        int len = data.length;
        int max_n = 12 + len * (16 + 3);
        byte recipe[] = new byte[max_n];
        int pos;

        // Send 10 zeroes followed by 1
        pos = 0;
        for (int i = 0; i < 10; i++) {
            recipe[pos++] = 1;
        }
        recipe[pos++] = 1;
        recipe[pos++] = 0;

        // Send actual data bytes, each followed by 0 and 1
        for (int i = 0; i < len; i++) {
            int v = data[i];
            for (int j = 0; j < 8; j++) {
                recipe[pos++] = 1;
                if ((v & 1) == 1)
                {
                    recipe[pos++] = 0;
                }
                v >>= 1;
            }
            recipe[pos++] = 1;
            recipe[pos++] = 1;
            recipe[pos++] = 0;
        }

        byte rval[] = new byte[pos];
        for (int i = 0; i < pos; i++)
        {
            rval[i] = recipe[i];
        }
        return rval;
    }


    protected void startFlashing(int data[])
    {
        m_timer = new Timer();
        Helper task = new Helper();

        byte recipe[] = makeRecipe(data);

        task.setup((CameraManager) getSystemService(Context.CAMERA_SERVICE), recipe);
        m_timer.schedule(task, 100, 10);
    }

    protected void stopFlashing()
    {
        m_timer.cancel();
        m_timer.purge();
    }
}

class Helper extends TimerTask
{
    protected boolean m_led_on;

    protected CameraManager m_camera_manager;
    protected String m_camera_id;
    protected byte m_recipe[];
    protected int m_pos;

    public void setup(CameraManager camera_manager, byte recipe[])
    {
        m_camera_manager = camera_manager;
        m_recipe = recipe;
        m_pos = 0;
        m_led_on = false;

        try {
            m_camera_id = m_camera_manager.getCameraIdList()[0];
            // Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void run()
    {
        if (m_recipe[m_pos] != 0)
        {
            m_led_on = !m_led_on;
            try {
                m_camera_manager.setTorchMode(m_camera_id, m_led_on);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        if (++m_pos >= m_recipe.length) {
            m_pos = 0;
        }
    }
}
