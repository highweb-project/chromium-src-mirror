package org.chromium.device.contact.worker;

import android.content.ContentProviderOperation;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.util.Log;

import org.chromium.device.contact.OnContactWorkerListener;
import org.chromium.device.contact.object.ContactObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 14.
 */
public class DeleteContactWorker extends AbstractContactWorker {

    public DeleteContactWorker(Context context, OnContactWorkerListener listener) {
        super(context, listener);
    }

    @Override
    public void run() {
        Log.d(TAG, "Start Delete Contact working");

        boolean hasError = false;
        ArrayList<ContactObjectAndroid> results = new ArrayList<>();
        try {
            String condition = "";
            switch(mFindOptions.mTarget) {
                case NAME:
                    condition = mFindOptions.mSearchConditions.mDisplayName;
                    break;
                case PHONE_NUMBER:
                    condition = mFindOptions.mSearchConditions.mPhoneNumber;
                    break;
                case EMAIL_ADDRESS:
                    condition = mFindOptions.mSearchConditions.mEmailAddress;
                    break;
            }

            Cursor contactsCursor = mContext.getContentResolver().query(
                    Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_FILTER_URI, condition),
                    null,
                    null,
                    null,
                    ContactsContract.CommonDataKinds.Contactables.LOOKUP_KEY
            );
            contactsCursor.moveToFirst();

            if(contactsCursor.getCount() == 0) {
                mListener.onWorkFinished(Error.SUCCESS, results);
                return;
            }

            String contactID = contactsCursor.getString(contactsCursor.getColumnIndex(ContactsContract.Contacts._ID));
            results.add(findContactByID(contactID));

            ArrayList<ContentProviderOperation> ops = new ArrayList<>();
            ops.add(ContentProviderOperation.newDelete(ContactsContract.RawContacts.CONTENT_URI)
                    .withSelection(
                            ContactsContract.RawContacts.CONTACT_ID + "=?",
                            new String[]{contactID})
                    .build());

            hasError = mContext.getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops).length != 1;
        } catch (RemoteException e) {
            hasError = true;
            e.printStackTrace();
        } catch (OperationApplicationException e) {
            hasError = true;
            e.printStackTrace();
        } finally {
            if(hasError) {
                mListener.onWorkFinished(Error.IO_ERROR, results);
            }
            else {
                mListener.onWorkFinished(Error.SUCCESS, results);
            }
        }
    }
}
