package org.chromium.device.contact.worker;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.text.TextUtils;
import android.util.Log;

import org.chromium.device.contact.OnContactWorkerListener;
import org.chromium.device.contact.object.ContactAddressAndroid;
import org.chromium.device.contact.object.ContactObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public class FindContactWorker extends AbstractContactWorker {

    public FindContactWorker(Context context, OnContactWorkerListener listener) {
        super(context, listener);
    }

    @Override
    public void run() {
        Log.d(TAG, "Start Find Contact working");

        boolean hasException = false;
        ArrayList<ContactObjectAndroid> results = new ArrayList<>();
        Cursor contactsFilterCursor = null;
        Cursor dataCursor = null;
        Cursor rawContactsCursor = null;
        try {
            String condition = "";
            switch (mFindOptions.mTarget) {
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

            contactsFilterCursor = mContext.getContentResolver().query(
                    Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_FILTER_URI, condition),
                    null,
                    null,
                    null,
                    CommonDataKinds.Contactables.LOOKUP_KEY
            );

            String prevLookupId = "";
            String currentLookupId;
            String contactID;
            contactsFilterCursor.moveToFirst();
            do {
                currentLookupId = contactsFilterCursor.getString(contactsFilterCursor.getColumnIndex(CommonDataKinds.Contactables.LOOKUP_KEY));

                if (currentLookupId.equals(prevLookupId))
                    continue;

                dataCursor = mContext.getContentResolver().query(
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

                contactID = contactsFilterCursor.getString(contactsFilterCursor.getColumnIndex(ContactsContract.Contacts._ID));
                result.mID = contactID;

                rawContactsCursor = mContext.getContentResolver().query(
                        ContactsContract.RawContacts.CONTENT_URI,
                        null,
                        ContactsContract.RawContacts.CONTACT_ID + " = ?",
                        new String[]{contactID},
                        null
                );
                rawContactsCursor.moveToFirst();

                result.mAccountName = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_NAME));
                result.mAccountType = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_TYPE));

                results.add(result);
            } while (contactsFilterCursor.moveToNext() && results.size() < mFindOptions.mMaxItem);
        }
        catch (Exception e) {
            hasException = true;
            e.printStackTrace();
        }
        finally {
            if(contactsFilterCursor != null)
                contactsFilterCursor.close();
            if(dataCursor != null)
                dataCursor.close();
            if(rawContactsCursor != null)
                rawContactsCursor.close();

            mListener.onWorkFinished(hasException?Error.IO_ERROR:Error.SUCCESS, results);
        }
    }
}
