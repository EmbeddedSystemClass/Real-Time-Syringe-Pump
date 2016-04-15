package aj.syringepump;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class BluetoothConnection extends AppCompatActivity {

    private static final int REQUEST_ENABLE_BT = 1;

    ListView devices;
    Button getMyDevice;
//    TextView status;
    ArrayList<String> pairedDeviceArrayList;
    ArrayAdapter<String> pairedDeviceAdapter;

    List<String> deviceName = new ArrayList<>();

    String status;

    @Override
    protected void onStart() {
        super.onStart();

        BluetoothAdapter bluetooth = BluetoothAdapter.getDefaultAdapter();

        if(bluetooth != null) {
            //continue with bluetooth setup.
        }

        if(bluetooth.isEnabled()) {
            //Enabled. Work with Bluetooth.

            String mydeviceaddress = bluetooth.getAddress();
            String mydevicename = bluetooth.getName();
            status = mydevicename + " :" + mydeviceaddress;

            getListOfBluetoothDevices();

            devices.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                    Toast.makeText(BluetoothConnection.this, "Connecting to "
                            + deviceName.get(position), Toast.LENGTH_LONG).show();
                }
            });

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

//      status = (TextView) findViewById(R.id.connectionStatus);

        getMyDevice = (Button) findViewById(R.id.deviceButton);
        devices = (ListView) findViewById(R.id.listViewBluetooth);



        getMyDevice.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(getApplicationContext(), status, Toast.LENGTH_LONG).show();
            }
        });
    }

    public void getListOfBluetoothDevices() {
        Set<BluetoothDevice> pairedDevices = BluetoothAdapter.getDefaultAdapter().getBondedDevices();
        if(pairedDevices.size() > 0) {
            pairedDeviceArrayList = new ArrayList<>();

            for(BluetoothDevice device : pairedDevices) {
                pairedDeviceArrayList.add(device.getName() + " : " + device.getAddress());
                deviceName.add(device.getName());
            }

            pairedDeviceAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, pairedDeviceArrayList);

            devices.setAdapter(pairedDeviceAdapter);

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
}
