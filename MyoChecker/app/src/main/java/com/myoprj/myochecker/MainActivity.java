package com.myoprj.myochecker;

import android.app.Activity;
import android.app.Application;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.devbrackets.android.exomedia.EMVideoView;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.net.Socket;

public class MainActivity extends AppCompatActivity implements  MediaPlayer.OnErrorListener{
    Socket socket;
    BufferedInputStream networkReader;
    EMVideoView emVideoView;
    Button connectButton;
    EditText ipText;
    Switch[] args;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        emVideoView = (EMVideoView)findViewById(R.id.video_view);
        connectButton = (Button)findViewById(R.id.connectButton);
        ipText = (EditText)findViewById(R.id.ipText);
        emVideoView.setOnErrorListener(this);
        args = new Switch[5];
        args[0] = (Switch)findViewById(R.id.Arg0);
        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    emVideoView.setVideoURI(Uri.parse("http://"+ipText.getText().toString()+":8080/test"));
                    emVideoView.start();
                    checkUpdate.start();
                    connectButton.setEnabled(false);
                }catch(Exception e){
                    Toast toast = Toast.makeText(getApplicationContext(), "Access Error", Toast.LENGTH_SHORT);
                }
            }
        });
    }

    @Override
    public boolean onError(MediaPlayer mp, int i, int j){
        Toast toast = Toast.makeText(getApplicationContext(), "Access Error", Toast.LENGTH_SHORT);
        connectButton.setEnabled(true);
        return false;
    }

    public void setSocket(String ip, int port) throws IOException {

        try {
            socket = new Socket(ip, port);
            networkReader = new BufferedInputStream(socket.getInputStream());
        } catch (IOException e) {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    private Thread checkUpdate = new Thread() {

        public void run() {
            byte[] str = new byte[25];
            try{
                setSocket(ipText.getText().toString(), 9000);
                while(true) {
                    Log.i("i", "start");
                    networkReader.read(str, 0, 25);
                    Data.Sensor sensor = Data.Sensor.parseFrom(str);
                    args[0].setText("Arg0: " + sensor.getArg0());
                    Log.i("i", "print!");
                }
            } catch (Exception e) {
                Log.e("e", "fuck");
                e.printStackTrace();
                Log.e("e", "printed line char num2: " + str.length);
            }
            try{
                socket.close();
            }catch (Exception e){
                Log.e("e", "Socket Close Error");
            }
        }
    };
}
