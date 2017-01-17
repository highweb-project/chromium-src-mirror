package org.chromium.device.contact.worker;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.util.Log;

import org.chromium.device.contact.OnContactWorkerListener;
import org.chromium.device.contact.object.ContactAddressAndroid;
import org.chromium.device.contact.object.ContactObjectAndroid;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 14.
 */
public class UpdateContactWorker extends AbstractContactWorker {

    public UpdateContactWorker(Context context, ContactObjectAndroid contact, OnContactWorkerListener listener) {
        super(context, listener);
        mContact = contact;
    }

    @Override
    public void run() {
        Log.d(TAG, "Start Update Contact working");
        mContact.dump();

        int insertedOptCnt = 0;
        int succeedOptCnt = 0;
        ArrayList<ContactObjectAndroid> results = new ArrayList<>();
        try {
            ArrayList<ContentProviderOperation> ops = new ArrayList<>();

            //Initialize a operation that create Contact's name data
            if(!mContact.mStructuredName.isEmpty() || !TextUtils.isEmpty(mContact.mDisplayName)) {
                ContentProviderOperation.Builder insertNameOptBuilder = ContentProviderOperation.newUpdate(ContactsContract.Data.CONTENT_URI);
                insertNameOptBuilder.withSelection(
                        ContactsContract.Data.CONTACT_ID + "=?" + " AND " + ContactsContract.Data.MIMETYPE + "=?",
                        new String[]{mContact.mID, ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE});
                if (!mContact.mStructuredName.isEmpty()) {
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mGivenName)) {
                        insertNameOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredName.GIVEN_NAME, mContact.mStructuredName.mGivenName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mMiddleName)) {
                        insertNameOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredName.MIDDLE_NAME, mContact.mStructuredName.mMiddleName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mFamilyName)) {
                        insertNameOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredName.FAMILY_NAME, mContact.mStructuredName.mFamilyName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mHonorificPrefix)) {
                        insertNameOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredName.PREFIX, mContact.mStructuredName.mHonorificPrefix);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredName.mHonorificSuffix)) {
                        insertNameOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredName.SUFFIX, mContact.mStructuredName.mHonorificSuffix);
                    }

                } else {
                    insertNameOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredName.DISPLAY_NAME, mContact.mDisplayName);
                }
                ops.add(insertNameOptBuilder.build());
                ++insertedOptCnt;
            }

            //Initialize a operation that create Contact's phone number data.
            if(!TextUtils.isEmpty(mContact.mPhoneNumber)) {
                ops.add(ContentProviderOperation.newUpdate(ContactsContract.Data.CONTENT_URI)
                        .withSelection(
                                ContactsContract.Data.CONTACT_ID + "=?" + " AND " + ContactsContract.Data.MIMETYPE + "=?",
                                new String[]{mContact.mID, ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE})
                        .withValue(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE)
                        .withValue(ContactsContract.CommonDataKinds.Phone.NUMBER, mContact.mPhoneNumber)
                        .withValue(ContactsContract.CommonDataKinds.Phone.TYPE, ContactsContract.CommonDataKinds.Phone.TYPE_MOBILE)
                        .build());

                ++insertedOptCnt;
            }

            //Initialize a operation that create Contact's email data.
            if(!TextUtils.isEmpty(mContact.mEmailAddress)) {
                ops.add(ContentProviderOperation.newUpdate(ContactsContract.Data.CONTENT_URI)
                        .withSelection(
                                ContactsContract.Data.CONTACT_ID + "=?" + " AND " + ContactsContract.Data.MIMETYPE + "=?",
                                new String[]{mContact.mID, ContactsContract.CommonDataKinds.Email.CONTENT_ITEM_TYPE})
                        .withValue(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Email.CONTENT_ITEM_TYPE)
                        .withValue(ContactsContract.CommonDataKinds.Email.ADDRESS, mContact.mEmailAddress)
                        .build());

                ++insertedOptCnt;
            }

            //Initialize a operation that create Contact's address data.
            if(!TextUtils.isEmpty(mContact.mAddress) || !mContact.mStructuredAddress.isEmpty()) {
                ContentProviderOperation.Builder insertAddressOptBuilder = ContentProviderOperation.newUpdate(ContactsContract.Data.CONTENT_URI);
                insertAddressOptBuilder.withSelection(
                        ContactsContract.Data.CONTACT_ID + "=?" + " AND " + ContactsContract.Data.MIMETYPE + "=?",
                        new String[]{mContact.mID, ContactsContract.CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE});
                insertAddressOptBuilder.withValue(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE);

                if(!TextUtils.isEmpty(mContact.mAddress)) {
                    insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS, mContact.mAddress);
                }
                else {
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mType)) {
                        int addressType;
                        String addressTypeName = "";
                        if(mContact.mStructuredAddress.mType.equals(ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_HOME)) {
                            addressType = ContactsContract.CommonDataKinds.StructuredPostal.TYPE_HOME;
                        }
                        else if(mContact.mStructuredAddress.mType.equals(ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_WORK)) {
                            addressType = ContactsContract.CommonDataKinds.StructuredPostal.TYPE_WORK;
                        }
                        else if(mContact.mStructuredAddress.mType.equals(ContactAddressAndroid.DEFAULT_ADDRESS_TYPE_OTHER)) {
                            addressType = ContactsContract.CommonDataKinds.StructuredPostal.TYPE_OTHER;
                        }
                        else {
                            addressType = ContactsContract.CommonDataKinds.StructuredPostal.TYPE_CUSTOM;
                            addressTypeName = mContact.mStructuredAddress.mType;
                        }

                        insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.TYPE, addressType);
                        if(!TextUtils.isEmpty(addressTypeName))
                            insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.LABEL, addressTypeName);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mStreetAddress)) {
                        insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.STREET, mContact.mStructuredAddress.mStreetAddress);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mLocality)) {
                        insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.CITY, mContact.mStructuredAddress.mLocality);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mRegion)) {
                        insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.REGION, mContact.mStructuredAddress.mRegion);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mPostalCode)) {
                        insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.POSTCODE, mContact.mStructuredAddress.mPostalCode);
                    }
                    if(!TextUtils.isEmpty(mContact.mStructuredAddress.mCountry)) {
                        insertAddressOptBuilder.withValue(ContactsContract.CommonDataKinds.StructuredPostal.COUNTRY, mContact.mStructuredAddress.mCountry);
                    }
                }

                ops.add(insertAddressOptBuilder.build());

                ++insertedOptCnt;
            }

            //Initialize a operation that create Contact's category data.
            if(!mContact.mNewCategories.isEmpty()) {
                ContentProviderOperation.Builder insertCategoriesOptBuilder = ContentProviderOperation.newUpdate(ContactsContract.Data.CONTENT_URI);
                insertCategoriesOptBuilder.withSelection(
                        ContactsContract.Data.CONTACT_ID + "=?" + " AND " + ContactsContract.Data.MIMETYPE + "=?",
                        new String[]{mContact.mID, ContactsContract.CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE});
                insertCategoriesOptBuilder.withValue(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE);
                for(String category : mContact.mNewCategories) {
                    if(!TextUtils.isEmpty(category))
                        insertCategoriesOptBuilder.withValue(ContactsContract.CommonDataKinds.GroupMembership.GROUP_ROW_ID, findGroupIDFromName(category, true));
                }
                ops.add(insertCategoriesOptBuilder.build());

                ++insertedOptCnt;
            }

            succeedOptCnt = mContext.getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops).length;

            Thread.sleep(500);

            if(insertedOptCnt == succeedOptCnt) {
              ContactObjectAndroid result = findContactByID(mContact.mID);

              if(result == null) {
                String id = findContactIDFromName(mContact.mDisplayName);
                result = findContactByID(id);
              }

              results.add(result);
            }
        }
        catch (NullPointerException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (OperationApplicationException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            if(insertedOptCnt == succeedOptCnt) {
                mListener.onWorkFinished(Error.SUCCESS, results);
            }
            else {
                mListener.onWorkFinished(Error.IO_ERROR, results);
            }
        }
    }
}
