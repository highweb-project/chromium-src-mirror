package org.chromium.device.contact;

import org.chromium.device.contact.object.ContactObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public interface OnContactResultListener {
    void onWorkFinished(ContactConstantsAndroid.Error error, ArrayList<ContactObjectAndroid> results, Object callerData);
}
