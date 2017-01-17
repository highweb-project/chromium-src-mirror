package org.chromium.device.messaging.object;

import org.chromium.device.messaging.MessagingConstantsAndroid.MessageFindTarget;
import org.chromium.device.messaging.MessagingConstantsAndroid.MessageType;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class MessageFindOptionsAndroid {

    public MessageType mType;
    public MessageFindTarget mTarget;
    public String mCondition;
    public int mMaxItem;

}
