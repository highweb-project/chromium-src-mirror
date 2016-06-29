package org.chromium.device.messaging.worker;

import android.content.Context;
import android.content.Intent;

import org.chromium.device.messaging.MessagingConstantsAndroid;
import org.chromium.device.messaging.OnMessagingWorkerListener;
import org.chromium.device.messaging.helper.AbstractMessagingHelper;
import org.chromium.device.messaging.helper.MessagingHelperFactory;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class SendMessageWorker extends AbstractMessagingWorker {

    public SendMessageWorker(Context context, OnMessagingWorkerListener listener) {
        super(context, listener);
    }

    @Override
    public void run() {
        Intent intent = MessagingHelperFactory.getHelper(mContext).createSendMessageIntent(mMessage);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        MessagingConstantsAndroid.Error result = MessagingConstantsAndroid.Error.SUCCESS;

        try {
            mContext.startActivity(intent);
        } catch(Exception e) {
          e.printStackTrace();
          result = MessagingConstantsAndroid.Error.NOT_SUPPORTED_ERROR;
        }

        mListener.onWorkFinished(result, null);
    }
}
