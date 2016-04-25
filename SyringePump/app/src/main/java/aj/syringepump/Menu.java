package aj.syringepump;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class Menu extends AppCompatActivity{

    Button sendButton;
    EditText userInput;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu);

        userInput = (EditText) findViewById(R.id.userInput);
        sendButton = (Button) findViewById(R.id.sendButton);

        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                byte[] bytesToSend = userInput.getText().toString().getBytes();

                BluetoothConnection.myThreadConnected.write(bytesToSend);

            }
        });
    }
}
