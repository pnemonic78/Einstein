// TODO FG Review

package com.newtonforever.einstein.utils.screen;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.DisplayMetrics;

import com.newtonforever.einstein.utils.debug.DebugUtils;

/**
 * @author matt
 * @brief Find the sizes of the Newton and the host screen.
 * <p>
 * This class uses the preferences to find the size of the Newton screen.
 * It also searches the Display Metrics database to initialize the Host screen size.
 */
public class ScreenDimensionsInitializer {

    private ScreenDimensionsInitializer() {
        // No instances, please;
    }

    public static void initScreenDimensions(Context context) {
        initHostScreenDimensions(context);
        initNewtonScreenDimensions(context);
    }

    /**
     * Initializes the host screen dimensions.
     */
    private static void initHostScreenDimensions(Context context) {
        if (null == context) {
            DebugUtils.appendLog("ScreenDimensionInitializer.initHostScreenDimensions: context is null");
            return;
        }
        final DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        ScreenDimensions.HOST_SCREEN_WIDTH = metrics.widthPixels;
        ScreenDimensions.HOST_SCREEN_HEIGHT = metrics.heightPixels;
        DebugUtils.appendLog("ScreenDimensionInitializer.initHostScreenDimensions: Host screen size is " + ScreenDimensions.HOST_SCREEN_WIDTH + "x" + ScreenDimensions.HOST_SCREEN_HEIGHT);
    }

    /**
     * Initializes the host screen dimensions.
     */
    public static void initNewtonScreenDimensions(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        final String value = prefs.getString("screenpresets", "320 x 480");
        DebugUtils.appendLog("ScreenDimensionInitializer.initNewtonScreenDimensions: Current preference is " + value);
        final int separatorPosition = value.indexOf("x");
        final String sw = value.substring(0, separatorPosition).trim();
        DebugUtils.appendLog("ScreenDimensionInitializer.initNewtonScreenDimensions: Width got from preference string is " + sw);
        String sh = value.substring(separatorPosition + 1, value.length()).trim();
        DebugUtils.appendLog("ScreenDimensionInitializer.initNewtonScreenDimensions: Height got from preference string is " + sh);
        ScreenDimensions.NEWTON_SCREEN_WIDTH = Integer.parseInt(sw);
        ScreenDimensions.NEWTON_SCREEN_HEIGHT = Integer.parseInt(sh);
        DebugUtils.appendLog("ScreenDimensionInitializer.initNewtonScreenDimensions: Newton window size is " + ScreenDimensions.NEWTON_SCREEN_WIDTH + "x" + ScreenDimensions.NEWTON_SCREEN_HEIGHT);
    }

}
