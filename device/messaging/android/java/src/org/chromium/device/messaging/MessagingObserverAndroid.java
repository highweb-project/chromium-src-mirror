package org.chromium.device.messaging;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.os.Build;

import org.chromium.device.messaging.object.MessageObjectAndroid;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class MessagingObserverAndroid extends BroadcastReceiver implements MessagingConstantsAndroid {

    public static final String ACTION = "android.provider.Telephony.SMS_RECEIVED";

    HashMap<Integer, OnMessagingResultListener> mListenerMap;
    HashMap<Integer, Object> mCallerDataMap;

    public MessagingObserverAndroid() {
        Log.d("chromium", "MessagingObserverAndroid()");
        mListenerMap = new HashMap<>();
        mCallerDataMap = new HashMap<>();
    }

    public void addListener(int observerId, OnMessagingResultListener listener, Object callerData) {
        mListenerMap.put(observerId, listener);
        mCallerDataMap.put(observerId, callerData);
    }

    public boolean removeListener(int observerId) {
        mListenerMap.remove(observerId);
        mCallerDataMap.remove(observerId);

        if(mListenerMap.size()>0)
            return false;
        else
            return true;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Bundle bundle = intent.getExtras();
        Object[] pdusObj = (Object[])bundle.get("pdus");
        String format = "";
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            format = bundle.getString("format");
        }
        ArrayList<MessageObjectAndroid> results = new ArrayList<>();
        for(int i=0; i<pdusObj.length; i++) {
            SmsMessage currentMessage;

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                currentMessage = SmsMessage.createFromPdu((byte[])pdusObj[i], format);
            } else {
                currentMessage = SmsMessage.createFromPdu((byte[])pdusObj[i]);
            }

            MessageObjectAndroid result = new MessageObjectAndroid();
            result.mType = MessageType.SMS;
            result.mTo = ((TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE)).getLine1Number();
            if (result.mTo == null) {
              result.mTo = "";
            }
            result.mFrom = currentMessage.getDisplayOriginatingAddress();
            if (result.mFrom == null) {
              result.mFrom = "";
            }
            try {
                result.mBody = new String(currentMessage.getDisplayMessageBody().replace("\n", "").getBytes(), "utf-8");
                if (result.mBody == null) {
                  result.mBody = "";
                }
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
                result.mBody = "";
            }
            result.mDate = Utils.getDateString(currentMessage.getTimestampMillis());

            results.add(result.copy());
        }

        Iterator<Integer> keys = mListenerMap.keySet().iterator();
        while(keys.hasNext()) {
            int key = keys.next();
            mListenerMap.get(key).onWorkFinished(Error.SUCCESS, results, mCallerDataMap.get(key));
        }

        mListenerMap.clear();
        mCallerDataMap.clear();
    }
}
