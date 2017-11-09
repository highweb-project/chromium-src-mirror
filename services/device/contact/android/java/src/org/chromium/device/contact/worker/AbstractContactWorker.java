package org.chromium.device.contact.worker;

import android.content.ContentProviderOperation;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds;
import android.text.TextUtils;
import android.util.Log;

import org.chromium.device.contact.ContactConstantsAndroid;
import org.chromium.device.contact.OnContactWorkerListener;
import org.chromium.device.contact.object.ContactAddressAndroid;
import org.chromium.device.contact.object.ContactFindOptionsAndroid;
import org.chromium.device.contact.object.ContactObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 14.
 */
abstract public class AbstractContactWorker implements Runnable, ContactConstantsAndroid {

    protected Context mContext;
    protected OnContactWorkerListener mListener;

    protected ContactObjectAndroid mContact;
    protected ContactFindOptionsAndroid mFindOptions;

    public AbstractContactWorker(Context context, OnContactWorkerListener listener) {
        mContext = context;
        mListener = listener;
    }

    public void setContactData(ContactObjectAndroid contact) {
        mContact = contact;
    }

    public void setContactFindOptions(ContactFindOptionsAndroid findOptions) {
        mFindOptions = findOptions;
    }

    protected String findGroupNameFromID(String id) {
        Cursor cursor = mContext.getContentResolver().query(
                ContactsContract.Groups.CONTENT_URI,
                null,
                ContactsContract.Groups._ID + " = ?",
                new String[] {id},
                null
        );
        cursor.moveToFirst();

        if(cursor.getCount() == 1) {
            return cursor.getString(cursor.getColumnIndex(ContactsContract.Groups.TITLE));
        }
        else {
            return null;
        }
    }

    protected String findGroupIDFromName(String title) {
        return findGroupIDFromName(title, false);
    }

    protected String findGroupIDFromName(String name, boolean forced) {
        Cursor cursor = mContext.getContentResolver().query(
                ContactsContract.Groups.CONTENT_URI,
                null,
                ContactsContract.Groups.TITLE + " = ?",
                new String[] {name},
                null
        );
        cursor.moveToFirst();

        if(cursor.getCount() == 1) {
            return cursor.getString(cursor.getColumnIndex(ContactsContract.Groups._ID));
        }
        else {
            if(!forced)
                return null;

            ArrayList<ContentProviderOperation> ops = new ArrayList<>();
            ops.add(ContentProviderOperation.newInsert(ContactsContract.Groups.CONTENT_URI)
                    .withValue(ContactsContract.Groups.TITLE, name)
                    .withValue(ContactsContract.Groups.ACCOUNT_NAME, mContact.mAccountName)
                    .withValue(ContactsContract.Groups.ACCOUNT_TYPE, mContact.mAccountType)
                    .build());
            try {
                mContext.getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops);
            } catch (RemoteException e) {
                e.printStackTrace();
                return null;
            } catch (OperationApplicationException e) {
                e.printStackTrace();
                return null;
            }

            return findGroupIDFromName(name);
        }
    }

    public String findContactIDFromName(String name) {
        Log.d(TAG, "findConactIDFromName, name : " + name);
        Cursor cursor = mContext.getContentResolver().query(
                ContactsContract.Data.CONTENT_URI,
                null,
                ContactsContract.CommonDataKinds.StructuredName.DISPLAY_NAME + " = ?",
                new String[] {name},
                null
        );
        cursor.moveToFirst();
        String result = null;

        if(cursor.getCount() >= 1) {
          int columnIndex = cursor.getColumnIndex("contact_id");
          String columnData = cursor.getString(columnIndex);
          Log.d(TAG, "columnIndex : " + columnIndex + ", columnData : " + columnData);

          result = columnData;
        }

        // StringBuffer buffer = new StringBuffer();
        // String[] columnNames = cursor.getColumnNames();
        //
        // for (String columnName : columnNames) {
        //   int columnIndex = cursor.getColumnIndex(columnName);
        //   String columnData = cursor.getString(columnIndex);
        //   buffer.append(columnName + "(" + columnIndex + ") : " + columnData + " / ");
        // }
        // Log.d(TAG, buffer.toString());
        // Log.d(TAG, "cursor.getCount() : " + cursor.getCount());

        cursor.close();
        return result;
    }

    ContactObjectAndroid findContactByID(String contactID) {
        Log.d("AbstractContactWorker", "findContactByID, contactID : " + contactID);

        Cursor cursor = mContext.getContentResolver().query(
                Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, contactID),
                null,
                null,
                null,
                null
        );

        Log.d("AbstractContactWorker", "findContactByID, cursor.getCount() : " + cursor.getCount());
        Log.d("AbstractContactWorker", "cursor.getColumnIndex(CommonDataKinds.Contactables.LOOKUP_KEY) : " +cursor.getColumnIndex(CommonDataKinds.Contactables.LOOKUP_KEY));

        if(cursor.getCount() == 0) {
          cursor.close();

          return null;
        }

        cursor.moveToFirst();

        String currentLookupId = cursor.getString(cursor.getColumnIndex(CommonDataKinds.Contactables.LOOKUP_KEY));

        cursor.close();

        Cursor dataCursor = mContext.getContentResolver().query(
                ContactsContract.Data.CONTENT_URI,
                null,
                ContactsContract.Data.LOOKUP_KEY + " = ?",
                new String[]{currentLookupId},
                ContactsContract.Data.MIMETYPE
        );

        ContactObjectAndroid result = new ContactObjectAndroid();
        dataCursor.moveToFirst();
        do {

            String mimeType = dataCursor.getString(dataCursor.getColumnIndex(ContactsContract.Data.MIMETYPE));

            if (mimeType.equals(CommonDataKinds.Phone.CONTENT_ITEM_TYPE)) {
                result.mPhoneNumber = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.Phone.NUMBER));
            } else if (mimeType.equals(CommonDataKinds.Email.CONTENT_ITEM_TYPE)) {
                result.mEmailAddress = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.Email.ADDRESS));
            } else if (mimeType.equals(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                result.mDisplayName = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredName.DISPLAY_NAME));
                result.mStructuredName.mGivenName = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredName.GIVEN_NAME));
                result.mStructuredName.mMiddleName = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredName.MIDDLE_NAME));
                result.mStructuredName.mFamilyName = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredName.FAMILY_NAME));
                result.mStructuredName.mHonorificPrefix = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredName.PREFIX));
                result.mStructuredName.mHonorificSuffix = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredName.SUFFIX));
            } else if (mimeType.equals(CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE)) {
                String groupName = findGroupNameFromID(dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.GroupMembership.GROUP_ROW_ID)));
                if (!TextUtils.isEmpty(groupName))
                    result.mNewCategories.add(groupName);
            } else if (mimeType.equals(CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE)) {
                result.mAddress = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS));

                String addressTypeName;
                int addressType = dataCursor.getInt(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.TYPE));
                if (addressType == CommonDataKinds.StructuredPostal.TYPE_HOME) {
                    addressTypeName = ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_HOME;
                } else if (addressType == CommonDataKinds.StructuredPostal.TYPE_WORK) {
                    addressTypeName = ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_WORK;
                } else if (addressType == CommonDataKinds.StructuredPostal.TYPE_OTHER) {
                    addressTypeName = ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_OTHER;
                } else {
                    addressTypeName = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.LABEL));
                }
                result.mStructuredAddress.mType = addressTypeName;
                result.mStructuredAddress.mStreetAddress = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.STREET));
                result.mStructuredAddress.mRegion = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.REGION));
                result.mStructuredAddress.mLocality = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.CITY));
                result.mStructuredAddress.mCountry = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.COUNTRY));
                result.mStructuredAddress.mPostalCode = dataCursor.getString(dataCursor.getColumnIndex(CommonDataKinds.StructuredPostal.POSTCODE));
            }

        } while (dataCursor.moveToNext());
        dataCursor.close();

        Cursor rawContactsCursor = mContext.getContentResolver().query(
                ContactsContract.RawContacts.CONTENT_URI,
                null,
                ContactsContract.RawContacts.CONTACT_ID + " = ?",
                new String[]{contactID},
                null
        );
        rawContactsCursor.moveToFirst();

        result.mID = contactID;
        result.mAccountName = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_NAME));
        result.mAccountType = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_TYPE));

        rawContactsCursor.close();

        return result;
    }
}
