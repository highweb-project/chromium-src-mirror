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

                if (mCursorHelper.getID() != null) {
                  messageObject.mID = mCursorHelper.getID();
                } else {
                  messageObject.mID = "";
                }
                messageObject.mType = MessageType.SMS;

                if (mCursorHelper.getTo() != null) {
                  messageObject.mTo = mCursorHelper.getTo();
                } else {
                  messageObject.mTo = "";
                }
                if (mCursorHelper.getFrom() != null) {
                  messageObject.mFrom = mCursorHelper.getFrom();
                } else {
                  messageObject.mFrom = "";
                }
                if (mCursorHelper.getTitle() != null) {
                  messageObject.mTitle = mCursorHelper.getTitle();
                } else {
                  messageObject.mTitle = "";
                }
                if (mCursorHelper.getBody() != null) {
                  messageObject.mBody = mCursorHelper.getBody();
                } else {
                  messageObject.mBody = "";
                }
                if (mCursorHelper.getDate() != null) {
                  messageObject.mDate = mCursorHelper.getDate();
                } else {
                  messageObject.mDate = "";
                }

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
