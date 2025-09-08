package com.shepamike.voicechanger

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Button
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat

class MainActivity : AppCompatActivity() {

    private external fun nativeStart()
    private external fun nativeStop()

    private val requestPermission = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ){ granted -> if (granted) nativeStart() }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        System.loadLibrary("voicechanger")

        findViewById<Button>(R.id.btnStart).setOnClickListener {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
                != PackageManager.PERMISSION_GRANTED) {
                requestPermission.launch(Manifest.permission.RECORD_AUDIO)
            } else nativeStart()
        }
        findViewById<Button>(R.id.btnStop).setOnClickListener { nativeStop() }
    }
}
