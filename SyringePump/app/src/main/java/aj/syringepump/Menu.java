package aj.syringepump;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class Menu extends AppCompatActivity{

    Button sendButton1;
    Button sendButton10;
    Button sendButton100;
    Button sendButton1000;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu);

        sendButton1 = (Button) findViewById(R.id.sendButton_1);
        sendButton10 = (Button) findViewById(R.id.sendButton_10);
        sendButton100 = (Button) findViewById(R.id.sendButton_100);
        sendButton1000 = (Button) findViewById(R.id.sendButton_1000);

        /* send 1 */
        sendButton1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send1Byte = "a".getBytes();
                BluetoothConnection.myThreadConnected.write(send1Byte);
                Toast toast = Toast.makeText(getApplicationContext(), "1 Steps", Toast.LENGTH_LONG);
                toast.show();
            }
        });

        /* send 10 */
        sendButton10.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send10Byte = "b".getBytes();
                BluetoothConnection.myThreadConnected.write(send10Byte);
                Toast toast = Toast.makeText(getApplicationContext(), "10 Steps", Toast.LENGTH_LONG);
                toast.show();
            }
        });

        /* send 100 */
        sendButton100.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send100Byte = "c".getBytes();
                BluetoothConnection.myThreadConnected.write(send100Byte);
                Toast toast = Toast.makeText(getApplicationContext(), "100 Steps", Toast.LENGTH_LONG);
                toast.show();
            }
        });

        /* send 1000 */
        sendButton1000.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send1000Byte = "d".getBytes();
                BluetoothConnection.myThreadConnected.write(send1000Byte);
                Toast toast = Toast.makeText(getApplicationContext(), "1000 Steps", Toast.LENGTH_LONG);
                toast.show();
            }
        });
    }

    @Override
    public void onBackPressed() {
        // Write your code here
            BluetoothConnection.myThreadConnected.cancel();
        super.onBackPressed();
    }
}
