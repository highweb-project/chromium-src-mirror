package org.chromium.device.messaging.worker;

import android.content.Context;

import org.chromium.device.messaging.OnMessagingWorkerListener;
import org.chromium.device.messaging.object.MessageFindOptionsAndroid;
import org.chromium.device.messaging.object.MessageObjectAndroid;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public abstract class AbstractMessagingWorker implements Runnable {


    protected Context mContext;
    protected OnMessagingWorkerListener mListener;

    protected MessageObjectAndroid mMessage;
    protected MessageFindOptionsAndroid mFindOptions;

    public AbstractMessagingWorker(Context context, OnMessagingWorkerListener listener) {
        mContext = context;
        mListener = listener;
    }

    public void setMessageData(MessageObjectAndroid contact) {
        mMessage = contact;
    }

    public void setMessageFindOptions(MessageFindOptionsAndroid findOptions) {
        mFindOptions = findOptions;
    }
}
