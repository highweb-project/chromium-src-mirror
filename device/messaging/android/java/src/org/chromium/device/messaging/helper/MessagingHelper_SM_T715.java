package org.chromium.device.messaging.helper;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.telephony.TelephonyManager;
import android.util.Log;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class MessagingHelper_SM_T715 extends AbstractMessagingHelper {

    private static final String CONTENT_URI_STR = "content://sms";
    private static final Uri CONTENT_URI = Uri.parse(CONTENT_URI_STR);
    private static final String FIELD_NAME_ID = "_id";
    private static final String FIELD_NAME_TO = "";
    private static final String FIELD_NAME_FROM = "address";
    private static final String FIELD_NAME_BODY = "body";
    private static final String FIELD_NAME_TITLE = "subject";
    private static final String FIELD_NAME_DATE = "date";

    private static final String EXTRA_NAME_TO = "address";
    private static final String EXTRA_NAME_BODY = "sms_body";

    public MessagingHelper_SM_T715(Context context) {
        super(context);
        Log.d("", "CursorHelper_SM_T715()");
    }

    @Override
    protected Uri getContentUri() {
        return CONTENT_URI;
    }

    @Override
    protected String getIDFiledName() {
        return FIELD_NAME_ID;
    }

    @Override
    protected String getToFiledName() {
        return FIELD_NAME_TO;
    }

    @Override
    protected String getFromFiledName() {
        return FIELD_NAME_FROM;
    }

    @Override
    protected String getTitleFiledName() {
        return FIELD_NAME_TITLE;
    }

    @Override
    protected String getBodyFiledName() {
        return FIELD_NAME_BODY;
    }

    @Override
    protected String getDateFiledName() {
        return FIELD_NAME_DATE;
    }

    @Override
    protected String getAddressNameForIntent() {
        return EXTRA_NAME_TO;
    }

    @Override
    protected String getBodyNameForIntent() {
        return EXTRA_NAME_BODY;
    }

    @Override
    public String getTo() {
        return ((TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE)).getLine1Number();
    }

    @Override
    public String getBody() {
        return super.getBody().replace("\n", "");
    }

    public void printAllColumName() {
        Cursor smsCursor = mContext.getContentResolver().query(
                CONTENT_URI,
                null,
                null,
                null,
                null
        );

        if(smsCursor.getCount() > 0) {
            for(int i=0; i<smsCursor.getColumnCount(); i++)
                Log.d("", smsCursor.getColumnName(i));
        }
        else {
            Log.d("", "no results...");
        }
    }
}
