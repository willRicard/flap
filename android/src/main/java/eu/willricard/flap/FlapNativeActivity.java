package eu.willricard.flap;

import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.DialogInterface;

public class FlapNativeActivity extends NativeActivity {
    public void failWithError(String error) {
        final String message = "An error has occured: " + error;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(FlapNativeActivity.this);
                AlertDialog dialog = builder.setMessage(message)
                        .setTitle("Error")
                        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                                finish();
                            }
                        })
                        .create();

                dialog.show();
            }
        });
    }
}
