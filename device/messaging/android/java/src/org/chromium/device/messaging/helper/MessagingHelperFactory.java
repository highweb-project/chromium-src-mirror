package org.chromium.device.messaging.helper;

import android.content.Context;
import android.os.Build;

import java.util.HashMap;
import java.util.Locale;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class MessagingHelperFactory {

    private static final String EMPTY_HELPER = "EMPTY";

    private static HashMap<String, AbstractMessagingHelper> mMessagingHelperMap = new HashMap<>();

    public static AbstractMessagingHelper getHelper(Context context) {
        if(mMessagingHelperMap.size() < 1) {
            //For unsupported device
            mMessagingHelperMap.put(EMPTY_HELPER, new EmptyMessagingHelper(context));
            //For SM-T715(Galaxy Tab S2)
            mMessagingHelperMap.put("SM-T715N0", new MessagingHelper_SM_T715(context));
        }

        AbstractMessagingHelper helper = mMessagingHelperMap.get(Build.MODEL.toUpperCase(Locale.getDefault()));
        if(helper == null)
            helper = mMessagingHelperMap.get(EMPTY_HELPER);

        return helper;
    }

}
