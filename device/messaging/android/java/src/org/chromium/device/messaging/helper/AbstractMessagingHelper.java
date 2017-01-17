package org.chromium.device.messaging.helper;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

import org.chromium.device.messaging.MessagingConstantsAndroid;
import org.chromium.device.messaging.Utils;
import org.chromium.device.messaging.object.MessageObjectAndroid;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public abstract class AbstractMessagingHelper implements MessagingConstantsAndroid {

    protected Context mContext;

    protected Cursor mFindResultCursor;

    public AbstractMessagingHelper(Context context) {
        mContext = context;
    }

    public Error createFindCursor(MessagingConstantsAndroid.MessageFindTarget target, String condition) throws Exception {

        StringBuilder selectionBuilder = new StringBuilder();
        switch(target) {
            case BODY:
                selectionBuilder.append(" instr("+getBodyFiledName()+", ?) > 0");
                break;
            case FROM:
                selectionBuilder.append(getFromFiledName());
                selectionBuilder.append(" = ?");
                break;
        }

        Log.d("", selectionBuilder.toString());

        mFindResultCursor = mContext.getContentResolver().query(
                getContentUri(),
                null,
                selectionBuilder.toString(),
                new String[] {condition},
                null
        );

        if(mFindResultCursor == null)
            return Error.IO_ERROR;
        else
            return Error.SUCCESS;
    }

    public int getCount() {
        if(mFindResultCursor != null)
            return mFindResultCursor.getCount();
        else
            return -1;
    }

    public boolean moveToFirst() {
        if(mFindResultCursor != null)
            return mFindResultCursor.moveToFirst();
        else
            return false;
    }

    public boolean moveToNext() {
        if(mFindResultCursor != null)
            return mFindResultCursor.moveToNext();
        else
            return false;
    }

    public boolean moveToPrevious() {
        if(mFindResultCursor != null)
            return mFindResultCursor.moveToPrevious();
        else
            return false;
    }

    public String getID() {
        if(mFindResultCursor == null || mFindResultCursor.getCount() < 1)
            return null;

        return mFindResultCursor.getString(mFindResultCursor.getColumnIndex(getIDFiledName()));
    }

    public String getTo() {
        if(mFindResultCursor == null || mFindResultCursor.getCount() < 1)
            return null;

        return mFindResultCursor.getString(mFindResultCursor.getColumnIndex(getToFiledName()));
    }

    public String getFrom() {
        if(mFindResultCursor == null || mFindResultCursor.getCount() < 1)
            return null;

        return mFindResultCursor.getString(mFindResultCursor.getColumnIndex(getFromFiledName()));
    }

    public String getTitle() {
        if(mFindResultCursor == null || mFindResultCursor.getCount() < 1)
            return null;

        return mFindResultCursor.getString(mFindResultCursor.getColumnIndex(getTitleFiledName()));
    }

    public String getBody() {
        if(mFindResultCursor == null || mFindResultCursor.getCount() < 1)
            return null;

        return mFindResultCursor.getString(mFindResultCursor.getColumnIndex(getBodyFiledName()));
    }

    public String getDate() {
        if(mFindResultCursor == null || mFindResultCursor.getCount() < 1)
            return null;

        return Utils.getDateString(mFindResultCursor.getLong(mFindResultCursor.getColumnIndex(getDateFiledName())));
    }

    abstract protected Uri getContentUri();
    abstract protected String getIDFiledName();
    abstract protected String getToFiledName();
    abstract protected String getFromFiledName();
    abstract protected String getTitleFiledName();
    abstract protected String getBodyFiledName();
    abstract protected String getDateFiledName();

    public Intent createSendMessageIntent(MessageObjectAndroid message) {
        Intent sendMessageIntent = new Intent(Intent.ACTION_VIEW);
        sendMessageIntent.putExtra(getAddressNameForIntent(), message.mTo);
        sendMessageIntent.putExtra(getBodyNameForIntent(), message.mBody);
        sendMessageIntent.setType("vnd.android-dir/mms-sms");

        return sendMessageIntent;
    }

    abstract protected String getAddressNameForIntent();
    abstract protected String getBodyNameForIntent();
}
