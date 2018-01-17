package com.microsoft.xplatappxandroid;

import android.content.res.AssetManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Prevent this to be garbage collected when the native call is executing.
    private AssetManager mgr;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        // Get assets resource manager
        mgr = getResources().getAssets();
        // getFilesDir creates a files directory if not present.
        // This seems fine to cleaner on where we copy the appxs
        tv.setText(RunTests(mgr, this.getFilesDir().toString()));
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String RunTests(AssetManager mgr, String storagePath);
}
