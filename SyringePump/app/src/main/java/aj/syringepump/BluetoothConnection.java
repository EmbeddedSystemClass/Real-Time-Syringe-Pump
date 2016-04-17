package aj.syringepump;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Set;
import java.util.UUID;

public class BluetoothConnection extends AppCompatActivity {

    private static final int REQUEST_ENABLE_BT = 1;

    ListView devices;
    ArrayList<BluetoothDevice> pairedDeviceArrayList;
    ArrayList<String> pairedDeviceNameArrayList;
    ArrayAdapter<BluetoothDevice> pairedDeviceAdapter;
    ArrayAdapter<String> pairedDeviceNameAdapter;

    BluetoothAdapter bluetooth = BluetoothAdapter.getDefaultAdapter();

    ConnectThread myThreadConnect;
    ConnectedThread myThreadConnected;

    String status;

    private UUID MY_UUID;

    @Override
    protected void onStart() {
        super.onStart();

        if(bluetooth != null) {
            //continue with bluetooth setup.
        }

        if(bluetooth.isEnabled()) {
            //Enabled. Work with Bluetooth.

            String mydeviceaddress = bluetooth.getAddress();
            String mydevicename = bluetooth.getName();
            status = mydevicename + " :" + mydeviceaddress;

            getListOfBluetoothDevices();

        }
        else
        {
            //Disabled. Do something else.
            status = "Bluetooth is not Enabled. Please enable Bluetooth.";
            Intent enableBluetooth = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBluetooth, REQUEST_ENABLE_BT);
        }

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bluetooth_connection);

        devices = (ListView) findViewById(R.id.listViewBluetooth);

        MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    }

    public void getListOfBluetoothDevices() {
        Set<BluetoothDevice> pairedDevices = BluetoothAdapter.getDefaultAdapter().getBondedDevices();
        if(pairedDevices.size() > 0) {
            pairedDeviceArrayList = new ArrayList<>();
            pairedDeviceNameArrayList = new ArrayList<>();

            for(BluetoothDevice device : pairedDevices) {
                pairedDeviceArrayList.add(device);
                pairedDeviceNameArrayList.add(device.getName());
            }

            pairedDeviceNameAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, pairedDeviceNameArrayList);

            devices.setAdapter(pairedDeviceNameAdapter);

            devices.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                if (devices.getItemAtPosition(position).equals("HC-06")) {
                    Intent gotoMenu = new Intent(getApplicationContext(), Menu.class);
                    startActivity(gotoMenu);

                    myThreadConnect = new ConnectThread(pairedDeviceArrayList.get(position));
                    myThreadConnect.start();

                    Toast toast = Toast.makeText(getApplicationContext(), "Connecting to "
                            + devices.getItemAtPosition(position), Toast.LENGTH_LONG);
                    toast.show();
                }

                else
                {
                    Toast toast = Toast.makeText(getApplicationContext(), devices.getItemAtPosition(position)
                            + " is not the bluetooth module for SJSUOneBoard", Toast.LENGTH_LONG);
                    toast.show();
                }
                }
            });

        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == REQUEST_ENABLE_BT){
            if(resultCode == Activity.RESULT_OK){
                getListOfBluetoothDevices();
            }else{
                Toast.makeText(this,
                        "BlueTooth NOT enabled",
                        Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }

    private void manageConnectedSocket(BluetoothSocket socket) {
        myThreadConnected = new ConnectedThread(socket);
        myThreadConnected.start();
    }

    private class ConnectThread extends Thread {
        // Use a temporary object that is later assigned to mmSocket,
        // because mmSocket is final
        private BluetoothSocket mmSocket = null;
        private final BluetoothDevice mmDevice;

        private ConnectThread(BluetoothDevice device) {
            BluetoothSocket tmp = null;
            mmDevice = device;

            // Get a BluetoothSocket to connect with the given BluetoothDevice
            try {
                // MY_UUID is the app's UUID string, also used by the server code
                tmp = device.createRfcommSocketToServiceRecord(MY_UUID);
            } catch (IOException e) {
            }
            mmSocket = tmp;
        }

        @Override
        public void run() {
            // Cancel discovery because it will slow down the connection
            //bluetooth.cancelDiscovery();

            try {
                // Connect the device through the socket. This will block
                // until it succeeds or throws an exception
                mmSocket.connect();
            } catch (IOException connectException) {
                // Unable to connect; close the socket and get out
                try {
                    mmSocket.close();
                } catch (IOException closeException) { }
                return;
            }

            // Do work to manage the connection (in a separate thread)
            manageConnectedSocket(mmSocket);
        }

        /** Will cancel an in-progress connection, and close the socket */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) { }
        }

    }

    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[1024];  // buffer store for the stream
            int bytes; // bytes returned from read()

            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer);
                    // Send the obtained bytes to the UI activity
                    //mHandler.obtainMessage(MESSAGE_READ, bytes, -1, buffer)
                    //        .sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }

        /* Call this from the main activity to send data to the remote device */
        public void write(byte[] bytes) {
            try {
                mmOutStream.write(bytes);
            } catch (IOException e) { }
        }

        /* Call this from the main activity to shutdown the connection */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) { }
        }
    }
}
