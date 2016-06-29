package org.chromium.device.messaging;

import org.chromium.device.messaging.object.MessageObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public interface OnMessagingResultListener {
    void onWorkFinished(MessagingConstantsAndroid.Error error, ArrayList<MessageObjectAndroid> results, Object callerData);
}
