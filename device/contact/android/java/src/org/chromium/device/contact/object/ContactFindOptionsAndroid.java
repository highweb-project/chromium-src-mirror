package org.chromium.device.contact.object;

import org.chromium.device.contact.ContactConstantsAndroid;
import org.chromium.device.contact.object.ContactObjectAndroid;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public class ContactFindOptionsAndroid implements ContactConstantsAndroid {

    public ContactFindOptionsAndroid() {
        mSearchConditions = new ContactObjectAndroid();
    }

    public ContactFindTarget mTarget;
    public int mMaxItem;
    public ContactObjectAndroid mSearchConditions;
}
