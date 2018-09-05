//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
package com.microsoft.androidbvt;

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
        System.loadLibrary("msix");
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
        tv.setText(RunTests(mgr, this.getFilesDir().toString()));

        // Terminate app
        this.finishAffinity();
        System.runFinalizersOnExit(true);
        System.exit(0);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String RunTests(AssetManager mgr, String storagePath);
}
