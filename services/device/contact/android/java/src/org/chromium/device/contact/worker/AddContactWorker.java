package org.chromium.device.contact.worker;

import android.content.ContentProviderOperation;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.*;
import android.text.TextUtils;
import android.util.Log;

import org.chromium.device.contact.OnContactWorkerListener;
import org.chromium.device.contact.object.ContactAddressAndroid;
import org.chromium.device.contact.object.ContactObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public class AddContactWorker extends AbstractContactWorker {

    public AddContactWorker(Context context, OnContactWorkerListener listener) {
        super(context, listener);
    }

    @Override
    public void run() {
        Log.d(TAG, "Start Add Contact working");
        mContact.dump();

        ArrayList<ContactObjectAndroid> results = new ArrayList<>();
        try {
            ContentValues values = new ContentValues();
            values.put(ContactsContract.RawContacts.ACCOUNT_TYPE, mContact.mAccountType);
            values.put(ContactsContract.RawContacts.ACCOUNT_NAME, mContact.mAccountName);
            Uri rawContactUri = mContext.getContentResolver().insert(ContactsContract.RawContacts.CONTENT_URI, values);

            long rawContactId = ContentUris.parseId(rawContactUri);

            ArrayList<ContentProviderOperation> ops = new ArrayList<>();

            //Initialize a operation that create Contact's name data
            if(!mContact.mStructuredName.isEmpty() || !TextUtils.isEmpty(mContact.mDisplayName)) {
                ContentProviderOperation.Builder insertNameOptBuilder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                insertNameOptBuilder.withValue(Data.RAW_CONTACT_ID, rawContactId);
                insertNameOptBuilder.withValue(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);
                if (!mContact.mStructuredName.isEmpty()) {
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mGivenName)) {
                        insertNameOptBuilder.withValue(StructuredName.GIVEN_NAME, mContact.mStructuredName.mGivenName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mMiddleName)) {
                        insertNameOptBuilder.withValue(StructuredName.MIDDLE_NAME, mContact.mStructuredName.mMiddleName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mFamilyName)) {
                        insertNameOptBuilder.withValue(StructuredName.FAMILY_NAME, mContact.mStructuredName.mFamilyName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mHonorificPrefix)) {
                        insertNameOptBuilder.withValue(StructuredName.PREFIX, mContact.mStructuredName.mHonorificPrefix);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mHonorificSuffix)) {
                        insertNameOptBuilder.withValue(StructuredName.SUFFIX, mContact.mStructuredName.mHonorificSuffix);
                    }
                } else {
                    insertNameOptBuilder.withValue(StructuredName.DISPLAY_NAME, mContact.mDisplayName);
                }
                ops.add(insertNameOptBuilder.build());
            }

            //Initialize a operation that create Contact's phone number data.
            if(!TextUtils.isEmpty(mContact.mPhoneNumber)) {
                ops.add(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                        .withValue(Data.RAW_CONTACT_ID, rawContactId)
                        .withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE)
                        .withValue(Phone.NUMBER, mContact.mPhoneNumber)
                        .withValue(Phone.TYPE, Phone.TYPE_MOBILE)
                        .build());
            }

            //Initialize a operation that create Contact's email data.
            if(!TextUtils.isEmpty(mContact.mEmailAddress)) {
                ops.add(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                        .withValue(Data.RAW_CONTACT_ID, rawContactId)
                        .withValue(Data.MIMETYPE, Email.CONTENT_ITEM_TYPE)
                        .withValue(Email.ADDRESS, mContact.mEmailAddress)
                        .build());
            }

            //Initialize a operation that create Contact's address data.
            if(!TextUtils.isEmpty(mContact.mAddress) || !mContact.mStructuredAddress.isEmpty()) {
                ContentProviderOperation.Builder insertAddressOptBuilder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                insertAddressOptBuilder.withValue(Data.RAW_CONTACT_ID, rawContactId);
                insertAddressOptBuilder.withValue(Data.MIMETYPE, StructuredPostal.CONTENT_ITEM_TYPE);

                if(!TextUtils.isEmpty(mContact.mAddress)) {
                    insertAddressOptBuilder.withValue(StructuredPostal.FORMATTED_ADDRESS, mContact.mAddress);
                }
                else {
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mType)) {
                        int addressType;
                        String addressTypeName = "";
                        if(mContact.mStructuredAddress.mType.equals(ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_HOME)) {
                            addressType = StructuredPostal.TYPE_HOME;
                        }
                        else if(mContact.mStructuredAddress.mType.equals(ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_WORK)) {
                            addressType = StructuredPostal.TYPE_WORK;
                        }
                        else if(mContact.mStructuredAddress.mType.equals(ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_OTHER)) {
                            addressType = StructuredPostal.TYPE_OTHER;
                        }
                        else {
                            addressType = StructuredPostal.TYPE_CUSTOM;
                            addressTypeName = mContact.mStructuredAddress.mType;
                        }

                        insertAddressOptBuilder.withValue(StructuredPostal.TYPE, addressType);
                        if(!TextUtils.isEmpty(addressTypeName))
                            insertAddressOptBuilder.withValue(StructuredPostal.LABEL, addressTypeName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mStreetAddress)) {
                        insertAddressOptBuilder.withValue(StructuredPostal.STREET, mContact.mStructuredAddress.mStreetAddress);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mLocality)) {
                        insertAddressOptBuilder.withValue(StructuredPostal.CITY, mContact.mStructuredAddress.mLocality);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mRegion)) {
                        insertAddressOptBuilder.withValue(StructuredPostal.REGION, mContact.mStructuredAddress.mRegion);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mPostalCode)) {
                        insertAddressOptBuilder.withValue(StructuredPostal.POSTCODE, mContact.mStructuredAddress.mPostalCode);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mCountry)) {
                        insertAddressOptBuilder.withValue(StructuredPostal.COUNTRY, mContact.mStructuredAddress.mCountry);
                    }
                }

                ops.add(insertAddressOptBuilder.build());
            }

            //Initialize a operation that create Contact's category data.
            if(!mContact.mNewCategories.isEmpty()) {
                ContentProviderOperation.Builder insertCategoriesOptBuilder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                insertCategoriesOptBuilder.withValue(Data.RAW_CONTACT_ID, rawContactId);
                insertCategoriesOptBuilder.withValue(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
                for(String category : mContact.mNewCategories) {
                    if(!TextUtils.isEmpty(category))
                        insertCategoriesOptBuilder.withValue(GroupMembership.GROUP_ROW_ID, findGroupIDFromName(category, true));
                }
                ops.add(insertCategoriesOptBuilder.build());
            }

            mContext.getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops);

            Thread.sleep(1000);

            Cursor rawContactsCursor = mContext.getContentResolver().query(
                    RawContacts.CONTENT_URI,
                    null,
                    RawContacts._ID + " = ?",
                    new String[]{String.valueOf(rawContactId)},
                    null
            );

            rawContactsCursor.moveToFirst();
            mContact.mID = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(RawContacts.CONTACT_ID));
            mContact.mAccountName = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(RawContacts.ACCOUNT_NAME));
            mContact.mAccountType = rawContactsCursor.getString(rawContactsCursor.getColumnIndex(RawContacts.ACCOUNT_TYPE));

            if(!mContact.mStructuredName.isEmpty()) {
                Cursor nameCursor = mContext.getContentResolver().query(
                        Data.CONTENT_URI,
                        null,
                        Data.RAW_CONTACT_ID + " = ? AND "+Data.MIMETYPE+"=?",
                        new String[]{String.valueOf(rawContactId), StructuredName.CONTENT_ITEM_TYPE},
                        null
                );
                nameCursor.moveToFirst();

                if(nameCursor.getCount() > 0) {
                    mContact.mDisplayName = nameCursor.getString(nameCursor.getColumnIndex(StructuredName.DISPLAY_NAME));
                }
            }

            results.add(mContact.copy());
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (OperationApplicationException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (SQLiteException e) {
            e.printStackTrace();
        } finally {
            mListener.onWorkFinished(results.size()>0?Error.SUCCESS:Error.IO_ERROR, results);
        }
    }
}
