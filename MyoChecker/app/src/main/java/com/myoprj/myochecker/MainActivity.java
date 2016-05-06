package com.myoprj.myochecker;

import android.app.Activity;
import android.app.Application;
import android.media.MediaPlayer;
import android.net.Uri;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.devbrackets.android.exomedia.EMVideoView;

public class MainActivity extends AppCompatActivity implements  MediaPlayer.OnErrorListener{
    EMVideoView emVideoView;
    Button connectButton;
    EditText ipText;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        emVideoView = (EMVideoView)findViewById(R.id.video_view);
        connectButton = (Button)findViewById(R.id.connectButton);
        ipText = (EditText)findViewById(R.id.ipText);
        emVideoView.setOnErrorListener(this);
        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    emVideoView.setVideoURI(Uri.parse(ipText.getText().toString()));
                    emVideoView.start();
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
}
