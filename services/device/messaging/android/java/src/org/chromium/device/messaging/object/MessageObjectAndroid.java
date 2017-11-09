package org.chromium.device.messaging.object;

import android.util.Log;

import org.chromium.device.messaging.MessagingConstantsAndroid.MessageType;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class MessageObjectAndroid {
    public String mID;
    public MessageType mType;
    public String mTo;
    public String mFrom;
    public String mTitle;
    public String mBody;
    public String mDate;

    public MessageObjectAndroid() {
        mID = "";
        mType = MessageType.NONE;
        mTo = "";
        mFrom = "";
        mTitle = "";
        mBody = "";
        mDate = "";
    }

    public void dump() {
        Log.d("chromium", "===================");
        Log.d("chromium", "MessageObjectAndroid DUMP");
        Log.d("chromium", "id="+mID);
        Log.d("chromium", "type="+mType);
        Log.d("chromium", "to="+mTo);
        Log.d("chromium", "from="+mFrom);
        Log.d("chromium", "title="+mTitle);
        Log.d("chromium", "body="+mBody);
        Log.d("chromium", "date="+mDate);
        Log.d("chromium", "===================");
    }

    public MessageObjectAndroid copy() {
        MessageObjectAndroid newObj = new MessageObjectAndroid();

        newObj.mID = this.mID;
        newObj.mType = this.mType;
        newObj.mTo = this.mTo;
        newObj.mFrom = this.mFrom;
        newObj.mTitle = this.mTitle;
        newObj.mBody = this.mBody;
        newObj.mDate = this.mDate;

        return newObj;
    }
}
