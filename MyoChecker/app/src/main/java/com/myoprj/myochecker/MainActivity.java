package com.myoprj.myochecker;


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
import android.widget.Toast;
import android.os.Handler;

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
    String[] textMsg;
    int i;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        emVideoView = (EMVideoView)findViewById(R.id.video_view);
        connectButton = (Button)findViewById(R.id.connectButton);
        textMsg = new String[5];
        ipText = (EditText)findViewById(R.id.ipText);
        emVideoView.setOnErrorListener(this);
        args = new Switch[5];
        i = 0;
        args[0] = (Switch)findViewById(R.id.Arg0);
        args[1] = (Switch)findViewById(R.id.Arg1);
        args[2] = (Switch)findViewById(R.id.Arg2);
        args[3] = (Switch)findViewById(R.id.Arg3);
        args[4] = (Switch)findViewById(R.id.Arg4);
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
    protected  void onStop(){
        super.onStop();
        emVideoView.release();

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

    public Handler mHandler = new Handler(){ // 핸들러 처리부분
        public void handleMessage(Message msg){ // 메시지를 받는부분
            args[0].setText(textMsg[0]);
            args[1].setText(textMsg[1]);
            args[2].setText(textMsg[2]);
            args[3].setText(textMsg[3]);
            args[4].setText(textMsg[4]);
        };
    };

    private Thread checkUpdate = new Thread() {

        public void run() {
            byte[] str = new byte[25];
            try{
                setSocket(ipText.getText().toString(), 9000);
                while(true) {
                    Log.i("i", "start");
                    networkReader.read(str, 0, 25);
                    Data.Sensor sensor = Data.Sensor.parseFrom(str);
                    textMsg[0] = "Arg0: " + sensor.getArg0();
                    textMsg[1] = "Arg1: " + sensor.getArg1();
                    textMsg[2] = "Arg2: " + sensor.getArg2();
                    textMsg[3] = "Arg3: " + sensor.getArg3();
                    textMsg[4] = "Arg4: " + sensor.getArg4();
                    Message msg = mHandler.obtainMessage();
                    mHandler.sendMessage(msg);
                    i++;
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
