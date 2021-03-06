package ch.pascal_mueller.frackstock;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.CountDownTimer;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class ConnectActivity extends AppCompatActivity {
    private static final int REQUEST_ENABLE_BT = 1;
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 2;

    private TextView textView_Info = null;

    private TextView tVBattery = null;
    private Button button1 = null;
    private Button button2 = null;
    private Button button3 = null;
    private Button button4 = null;
    private Button button5 = null;
    private Button button6 = null;
    private Button button7 = null;

    private BluetoothAdapter bluetoothAdapter = null;
    private BluetoothLeScanner bluetoothLeScanner = null;
    private BluetoothDevice bluetoothDevice = null;
    private UartService uartService = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setLayoutConnect();

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        service_init();

        if(bluetoothAdapter == null)
        {
            Toast.makeText(this, "Bluetooth is not available!", Toast.LENGTH_LONG).show();
            finish();
            return;
        }



        if (this.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("This app needs location access");
            builder.setMessage("Please grant location access so this app can use Bluetooth LE.");
            builder.setPositiveButton(android.R.string.ok, null);
            builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSION_REQUEST_COARSE_LOCATION);
                }
            });
            builder.show();
        }
        else {
            if (!bluetoothAdapter.isEnabled()) {
                Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
            }
            else {
                new CountDownTimer(500, 1) {
                    public void onTick(long l) {
                    }

                    public void onFinish() {
                        startConnecting();
                    }
                }.start();
            }
        }
    }

    private void setLayoutConnect()
    {
        setContentView(R.layout.activity_connect);
        textView_Info = (TextView) findViewById(R.id.textView1);

        button1 = null;
        button2 = null;
        button3 = null;
        button4 = null;
        button5 = null;
        button6 = null;
        button7 = null;
    }

    private void setLayoutControl()
    {
        setContentView(R.layout.activity_control);
        textView_Info = null;

        tVBattery = (TextView) findViewById(R.id.tVBattery);

        button1 = (Button) findViewById(R.id.button1);
        button2 = (Button) findViewById(R.id.button2);
        button3 = (Button) findViewById(R.id.button3);
        button4 = (Button) findViewById(R.id.button4);
        button5 = (Button) findViewById(R.id.button5);
        button6 = (Button) findViewById(R.id.button6);
        button7 = (Button) findViewById(R.id.button7);

        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x03, 0x01};    // enable master mode
                uartService.writeRXCharacteristic(value);
            }
        });

        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x03, 0x00};    // enable slave mode
                uartService.writeRXCharacteristic(value);
            }
        });

        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x02, 0x01};    // enable slave mode
                uartService.writeRXCharacteristic(value);
            }
        });

        button4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x02, 0x02};    // enable slave mode
                uartService.writeRXCharacteristic(value);
            }
        });

        button5.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x02, 0x03};    // enable slave mode
                uartService.writeRXCharacteristic(value);
            }
        });

        button6.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x02, 0x04};    // enable slave mode
                uartService.writeRXCharacteristic(value);
            }
        });

        button7.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] value = {0x02, 0x05};    // enable slave mode
                uartService.writeRXCharacteristic(value);
            }
        });
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode)
        {
            case PERMISSION_REQUEST_COARSE_LOCATION:
            {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    if (!bluetoothAdapter.isEnabled()) {
                        Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                        startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
                    }
                    else
                    {
                        startConnecting();
                    }
                }
                else
                {
                    Toast.makeText(this, "BluetoothLE is not available!", Toast.LENGTH_LONG).show();
                    finish();
                }
            }
        }

        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        switch (requestCode)
        {
            case REQUEST_ENABLE_BT:
                if(resultCode == RESULT_OK)
                    startConnecting();
                else
                    finish();
                break;
            default:
                break;
        }
    }

    private void startConnecting()
    {
        bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();

        if(bluetoothLeScanner != null)
        {
            textView_Info.setText("Searching for Frackstock with high RSSI.\nHold your Smartphone next to your Stick...");
            bluetoothLeScanner.startScan(scanCallback);
        }
        else
        {
            Toast.makeText(this, "BluetoothLE is not available!", Toast.LENGTH_LONG).show();
            finish();
        }
    }

    private ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if(result.getRssi() >= -55)
            {
                String dev_name = result.getDevice().getName();
                if(dev_name!= null && dev_name.compareTo("Frackstock") == 0) {
                    String message = "Found " + result.getDevice().getAddress()+ ", connecting...";
                    textView_Info.setText(message);
                    bluetoothLeScanner.stopScan(scanCallback);

                    bluetoothDevice = result.getDevice();
                    uartService.connect(bluetoothDevice.getAddress());
                }
            }
            super.onScanResult(callbackType, result);
        }
    };

    private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, final Intent intent) {
            String action = intent.getAction();

            final Intent mIntent = intent;
            //*********************//
            if (action.equals(UartService.ACTION_GATT_CONNECTED)) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        String message = "Connected to Frackstock (" + bluetoothDevice.getAddress() + ")";
                        textView_Info.setText(message);

                        new CountDownTimer(500, 1) {
                            public void onTick(long l) {
                            }

                            public void onFinish() {
                                //setLayoutControl();
                                startActivity(new Intent(getBaseContext(), MainControlActivity.class));

                                //setContentView(R.layout.activity_control);


                            }
                        }.start();

                        //String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        //Log.d(TAG, "UART_CONNECT_MSG");
                        //btnConnectDisconnect.setText("Disconnect");
                        //edtMessage.setEnabled(true);
                        //btnSend.setEnabled(true);
                        //((TextView) findViewById(R.id.deviceName)).setText(mDevice.getName()+ " - ready");
                        //listAdapter.add("["+currentDateTimeString+"] Connected to: "+ mDevice.getName());
                        //messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);
                        //mState = UART_PROFILE_CONNECTED;
                    }
                });
            }

            //*********************//
            if (action.equals(UartService.ACTION_GATT_DISCONNECTED)) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        //String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        /*Log.d(TAG, "UART_DISCONNECT_MSG");
                        btnConnectDisconnect.setText("Connect");
                        edtMessage.setEnabled(false);
                        btnSend.setEnabled(false);
                        ((TextView) findViewById(R.id.deviceName)).setText("Not Connected");
                        listAdapter.add("["+currentDateTimeString+"] Disconnected to: "+ mDevice.getName());
                        mState = UART_PROFILE_DISCONNECTED;*/
                        uartService.close();
                        setLayoutConnect();
                        textView_Info.setText("Disconnected.");

                        new CountDownTimer(500, 1) {
                            public void onTick(long l) {
                            }

                            public void onFinish() {
                                startConnecting();
                            }
                        }.start();
                        //setUiState();

                    }
                });
            }


            //*********************//
            if (action.equals(UartService.ACTION_GATT_SERVICES_DISCOVERED)) {
                uartService.enableTXNotification();
            }
            //*********************//
            if (action.equals(UartService.ACTION_DATA_AVAILABLE)) {

                final byte[] txValue = intent.getByteArrayExtra(UartService.EXTRA_DATA);
                runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            switch (txValue[0])
                            {
                                case 0x01:
                                    tVBattery.setText("Battery: " + txValue[1]);
                                    break;
                            }
                            //String text = new String(txValue, "UTF-8");
                            //String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                            /*listAdapter.add("["+currentDateTimeString+"] RX: "+text);
                            messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);*/

                        } catch (Exception e) {
                            //Log.e(TAG, e.toString());
                        }
                    }
                });
            }
            //*********************//
            if (action.equals(UartService.DEVICE_DOES_NOT_SUPPORT_UART)){
                //showMessage("Device doesn't support UART. Disconnecting");
                uartService.disconnect();
                startConnecting();
            }


        }
    };

    private void service_init() {
        Intent bindIntent = new Intent(this, UartService.class);
        bindService(bindIntent, mServiceConnection, Context.BIND_AUTO_CREATE);

        LocalBroadcastManager.getInstance(this).registerReceiver(UARTStatusChangeReceiver, makeGattUpdateIntentFilter());
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(UartService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(UartService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(UartService.DEVICE_DOES_NOT_SUPPORT_UART);
        return intentFilter;
    }

    //UART service connected/disconnected
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            uartService = ((UartService.LocalBinder) rawBinder).getService();
            //Log.d(TAG, "onServiceConnected mService= " + mService);
            if (!uartService.initialize()) {
                //Log.e(TAG, "Unable to initialize Bluetooth");
                finish();
            }

        }

        public void onServiceDisconnected(ComponentName classname) {
            ////     mService.disconnect(mDevice);
            uartService = null;
        }
    };

    @Override
    public void onDestroy() {
        super.onDestroy();
        //Log.d(TAG, "onDestroy()");

        try {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(UARTStatusChangeReceiver);
        } catch (Exception ignore) {
            //Log.e(TAG, ignore.toString());
        }
        unbindService(mServiceConnection);
        uartService.stopSelf();
        uartService= null;
    }
}
