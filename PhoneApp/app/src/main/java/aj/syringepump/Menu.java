package aj.syringepump;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class Menu extends AppCompatActivity{

    Button dispense;
    Button left;
    Button right;
    int num;
    int numOfSteps;
    boolean dis;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu);

        dispense = (Button) findViewById(R.id.dispenseButton);
        left = (Button) findViewById(R.id.leftBtn);
        right = (Button) findViewById(R.id.rightBtn);

        final EditText input = (EditText) findViewById(R.id.dispenseInput);

        left.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send1Byte = "e".getBytes();
                BluetoothConnection.myThreadConnected.write(send1Byte);
                Toast toast = Toast.makeText(getApplicationContext(), "Pumping Liquid!", Toast.LENGTH_LONG);
                toast.show();
            }
        });

        right.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] send1Byte = "f".getBytes();
                BluetoothConnection.myThreadConnected.write(send1Byte);
                Toast toast = Toast.makeText(getApplicationContext(), "Dispensing Liquid!", Toast.LENGTH_LONG);
                toast.show();
            }
        });

        /* Convert input to number of steps
         * There are 13000 steps in 30 ml
         * (30/13000) = mlPerSteps
         * 1/mlPerSteps = steps in 1 ml, which is 433 steps
         */

        /* When dispense button is pressed */

        dispense.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(input.length() > 0) {
                    String numString = input.getText().toString();
                    num = Integer.parseInt(numString);

                    if(num > 30)
                    {
                        dis = false;
                    }

                    else
                    {
                        dis = true;
                    }

                    numOfSteps = num * 433;
                    Toast toast1 = Toast.makeText(getApplicationContext(), "Number of Steps is: " + numOfSteps, Toast.LENGTH_LONG);
                    toast1.show();

                    while (numOfSteps > 0 && true) {
                        if (numOfSteps > 0 && numOfSteps < 10) {
                            byte[] send1Byte = "a".getBytes();
                            BluetoothConnection.myThreadConnected.write(send1Byte);
                            numOfSteps = numOfSteps - 1;
//                            Toast toast = Toast.makeText(getApplicationContext(), "1 Steps", Toast.LENGTH_LONG);
//                            toast.show();
                        }

                        if (numOfSteps > 9 && numOfSteps < 100) {
                            byte[] send10Byte = "b".getBytes();
                            BluetoothConnection.myThreadConnected.write(send10Byte);
                            numOfSteps = numOfSteps - 10;
//                            Toast toast = Toast.makeText(getApplicationContext(), "1 Steps", Toast.LENGTH_LONG);
//                            toast.show();
                        }
                        if (numOfSteps > 99 && numOfSteps < 1000) {
                            byte[] send100Byte = "c".getBytes();
                            BluetoothConnection.myThreadConnected.write(send100Byte);
                            numOfSteps = numOfSteps - 100;
//                            Toast toast = Toast.makeText(getApplicationContext(), "1 Steps", Toast.LENGTH_LONG);
//                            toast.show();
                        }
                        if (numOfSteps > 999) {
                            byte[] send1000Byte = "d".getBytes();
                            BluetoothConnection.myThreadConnected.write(send1000Byte);
                            numOfSteps = numOfSteps - 1000;
//                            Toast toast = Toast.makeText(getApplicationContext(), "1 Steps", Toast.LENGTH_LONG);
//                            toast.show();
                        }

                    }
                }

                else
                {
                    Toast toast = Toast.makeText(getApplicationContext(), "Enter an input", Toast.LENGTH_LONG);
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
