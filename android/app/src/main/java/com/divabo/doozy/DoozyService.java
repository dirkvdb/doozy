package com.divabo.doozy;

import android.app.IntentService;
import android.content.Intent;
import android.content.Context;
import android.util.Log;

/**
 * An {@link IntentService} subclass for handling asynchronous task requests in
 * a service on a separate handler thread.
 * <p/>
 * TODO: Customize class - update intent actions, extra parameters and static
 * helper methods.
 */
public class DoozyService extends IntentService {
    private static final String TAG = "DoozyService";
    private static final String ACTION_START = "com.divabo.doozy.action.START";
    private static final String ACTION_UPDATE_NAME = "com.divabo.doozy.action.UPDATE_NAME";

    private static final String DEVICE_NAME = "com.divabo.doozy.extra.DEVICE_NAME";

    /**
     * Starts this service to perform action Foo with the given parameters. If
     * the service is already performing a task this action will be queued.
     *
     * @see IntentService
     */
    // TODO: Customize helper method
    public static void startActionStart(Context context, String param1, String param2) {
        Log.i(TAG, "startActionStart");
        Intent intent = new Intent(context, DoozyService.class);
        intent.setAction(ACTION_START);
        context.startService(intent);
    }

    /**
     * Starts this service to perform action Baz with the given parameters. If
     * the service is already performing a task this action will be queued.
     *
     * @see IntentService
     */
    // TODO: Customize helper method
    public static void startActionUpdateName(Context context, String param1, String param2) {
        Intent intent = new Intent(context, DoozyService.class);
        intent.setAction(ACTION_UPDATE_NAME);
        intent.putExtra(DEVICE_NAME, param1);
        context.startService(intent);
    }

    public DoozyService() {
        super("DoozyService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (intent != null) {
            final String action = intent.getAction();
            if (ACTION_START.equals(action)) {
                final String param1 = intent.getStringExtra(DEVICE_NAME);
                handleActionStart(param1);
            } else if (ACTION_UPDATE_NAME.equals(action)) {
                final String param1 = intent.getStringExtra(DEVICE_NAME);
                handleActionUpdateName(param1);
            }
        }
    }

    /**
     * Handle action Foo in the provided background thread with the provided
     * parameters.
     */
    private void handleActionStart(String name) {
        Log.i(TAG, "Start service");
        throw new UnsupportedOperationException("Not yet implemented");
    }

    /**
     * Handle action Baz in the provided background thread with the provided
     * parameters.
     */
    private void handleActionUpdateName(String name) {
        throw new UnsupportedOperationException("Not yet implemented");
    }

    static {
        //load the doozy library
        System.loadLibrary("doozylib");
        System.loadLibrary("doozydroid");
    }
}
