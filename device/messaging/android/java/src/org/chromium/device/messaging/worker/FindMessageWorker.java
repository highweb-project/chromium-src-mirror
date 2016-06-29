package org.chromium.device.messaging.worker;

import android.content.Context;

import org.chromium.device.messaging.MessagingConstantsAndroid;
import org.chromium.device.messaging.OnMessagingWorkerListener;
import org.chromium.device.messaging.helper.AbstractMessagingHelper;
import org.chromium.device.messaging.helper.MessagingHelperFactory;
import org.chromium.device.messaging.object.MessageObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class FindMessageWorker extends AbstractMessagingWorker implements MessagingConstantsAndroid {

    AbstractMessagingHelper mCursorHelper;

    public FindMessageWorker(Context context, OnMessagingWorkerListener listener) {
        super(context, listener);
        mCursorHelper = MessagingHelperFactory.getHelper(mContext);
    }

    @Override
    public void run() {
        Error error = Error.SUCCESS;
        ArrayList<MessageObjectAndroid> results = new ArrayList<>();
        try {
            error = mCursorHelper.createFindCursor(mFindOptions.mTarget, mFindOptions.mCondition);
            if(error != Error.SUCCESS || mCursorHelper.getCount() < 1) {
                return;
            }

            MessageObjectAndroid messageObject;
            mCursorHelper.moveToFirst();
            do {
                messageObject = new MessageObjectAndroid();

                messageObject.mID = mCursorHelper.getID();
                messageObject.mType = MessageType.SMS;
                messageObject.mTo = mCursorHelper.getTo();
                messageObject.mFrom = mCursorHelper.getFrom();
                messageObject.mTitle = mCursorHelper.getTitle();
                messageObject.mBody = mCursorHelper.getBody();
                messageObject.mDate = mCursorHelper.getDate();

                results.add(messageObject.copy());
            }while(results.size() < mFindOptions.mMaxItem && mCursorHelper.moveToNext());
        } catch (Exception e) {
            error = Error.IO_ERROR;
            e.printStackTrace();
        } finally {
            mListener.onWorkFinished(error, results);
        }
    }
}
