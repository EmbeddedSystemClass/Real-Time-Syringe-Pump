package aj.syringepump;

import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class Menu extends AppCompatActivity{

    Button dispense;
    Button left;
    Button right;
    Button startBtn;
    Button doneBtn;

    TextView status;
    TextView direction;
    TextView numOfml;

    double num;
    double numOfSteps;
    boolean dis;

    Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu);

        startBtn = (Button) findViewById(R.id.startButton);
        doneBtn = (Button) findViewById(R.id.doneButton);
        dispense = (Button) findViewById(R.id.dispenseButton);
        left = (Button) findViewById(R.id.leftBtn);
        right = (Button) findViewById(R.id.rightBtn);

        status = (TextView) findViewById(R.id.activeText);
        direction = (TextView) findViewById(R.id.direction);
        numOfml = (TextView) findViewById(R.id.numOfml);

        final EditText input = (EditText) findViewById(R.id.dispenseInput);

        /* displays default values when app turns on */
        dispense.setEnabled(false);
        status.setText("inactive");
        direction.setText("outward");

        /* when startSession button is pressed */
        startBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dispense.setEnabled(true);
                status.setText("active");
            }
        });

        /* when endSession button is pressed */
        doneBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] sendoByte = "o".getBytes();
                BluetoothConnection.myThreadConnected.write(sendoByte);
                dispense.setEnabled(false);
                status.setText("inactive");
            }
        });

        /* when left button is pressed */
        left.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send1Byte = "e".getBytes();
                BluetoothConnection.myThreadConnected.write(send1Byte);
                direction.setText("inward");
            }
        });

        /* when right button is pressed */
        right.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send1Byte = "f".getBytes();
                BluetoothConnection.myThreadConnected.write(send1Byte);
                direction.setText("outward");
            }
        });

        /* When dispense button is pressed */
        dispense.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(input.length() > 0) {
                    String numString = input.getText().toString();
                    num = Float.valueOf(numString);

                    if(num > 30)
                    {
                        dis = false;
                    }

                    else
                    {
                        dis = true;
                        byte[] sendxByte = "x".getBytes();
                        BluetoothConnection.myThreadConnected.write(sendxByte);
                    }

                    /* Convert input to number of steps
                     * There are 13000 steps in 30 ml
                     * (30/13000) = mlPerSteps
                     * 1/mlPerSteps = steps in 1 ml, which is 433 steps
                     */

                    numOfSteps = Math.floor(num * 433);

                    while (numOfSteps > 0 && dis) {
                        if (numOfSteps > 0 && numOfSteps < 10) {
                            byte[] send1Byte = "a".getBytes();
                            BluetoothConnection.myThreadConnected.write(send1Byte);
                            numOfSteps = numOfSteps - 1;
                            Toast toast = Toast.makeText(getApplicationContext(), "# of Steps: " + numOfSteps, Toast.LENGTH_LONG);
                            toast.show();
                        }
                        else if (numOfSteps > 9 && numOfSteps < 100) {
                            byte[] send10Byte = "b".getBytes();
                            BluetoothConnection.myThreadConnected.write(send10Byte);
                            numOfSteps = numOfSteps - 10;
                            Toast toast = Toast.makeText(getApplicationContext(), "# of Steps: " + numOfSteps, Toast.LENGTH_LONG);
                            toast.show();
                        }
                        else if (numOfSteps > 99 && numOfSteps < 1000) {
                            byte[] send100Byte = "c".getBytes();
                            BluetoothConnection.myThreadConnected.write(send100Byte);
                            numOfSteps = numOfSteps - 100;
                            Toast toast = Toast.makeText(getApplicationContext(), "# of Steps: " + numOfSteps, Toast.LENGTH_LONG);
                            toast.show();
                        }
                        else if (numOfSteps > 999) {
                            byte[] send1000Byte = "d".getBytes();
                            BluetoothConnection.myThreadConnected.write(send1000Byte);
                            numOfSteps = numOfSteps - 1000;
                            Toast toast = Toast.makeText(getApplicationContext(), "# of Steps: " + numOfSteps, Toast.LENGTH_LONG);
                            toast.show();
                        }
                    }
                    byte[] sendyByte = "y".getBytes();
                    BluetoothConnection.myThreadConnected.write(sendyByte);
                }

                else
                {
                    Toast toast = Toast.makeText(getApplicationContext(), "Invalid Input!", Toast.LENGTH_LONG);
                    toast.show();
                }
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
