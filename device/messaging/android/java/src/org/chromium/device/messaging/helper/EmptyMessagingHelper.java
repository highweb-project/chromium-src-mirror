package org.chromium.device.messaging.helper;

import android.content.Context;
import android.net.Uri;

import org.chromium.device.messaging.MessagingConstantsAndroid;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class EmptyMessagingHelper extends AbstractMessagingHelper {

    public EmptyMessagingHelper(Context context) {
        super(context);
    }

    @Override
    public Error createFindCursor(MessagingConstantsAndroid.MessageFindTarget target, String condition) {
        return Error.NOT_SUPPORTED_ERROR;
    }

    @Override
    public String getID() {
        return null;
    }

    @Override
    public String getTo() {
        return null;
    }

    @Override
    public String getFrom() {
        return null;
    }

    @Override
    public String getTitle() {
        return null;
    }

    @Override
    public String getBody() {
        return null;
    }

    @Override
    public String getDate() {
        return null;
    }

    @Override
    protected Uri getContentUri() {
        return null;
    }

    @Override
    protected String getIDFiledName() {
        return null;
    }

    @Override
    protected String getToFiledName() {
        return null;
    }

    @Override
    protected String getFromFiledName() {
        return null;
    }

    @Override
    protected String getTitleFiledName() {
        return null;
    }

    @Override
    protected String getBodyFiledName() {
        return null;
    }

    @Override
    protected String getDateFiledName() {
        return null;
    }

    @Override
    protected String getAddressNameForIntent() {
        return null;
    }

    @Override
    protected String getBodyNameForIntent() {
        return null;
    }
}
