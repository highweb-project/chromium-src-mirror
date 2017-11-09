package org.chromium.device.contact.object;

import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public class ContactObjectAndroid {
    public String mID;
    public String mDisplayName;
    public ContactNameAndroid mStructuredName;
    public String mPhoneNumber;
    public String mEmailAddress;
    public String mAddress;
    public ContactAddressAndroid mStructuredAddress;
    //TODO should deprecate this member variable
    public String mCategories;
    public ArrayList<String> mNewCategories;
    public String mAccountName;
    public String mAccountType;

    public ContactObjectAndroid() {
        mID = "";
        mDisplayName = "";
        mPhoneNumber = "";
        mEmailAddress = "";
        mAddress = "";
        mCategories = "";
        mNewCategories = new ArrayList<>();
        mAccountName = "";
        mAccountType = "";
        mStructuredName = new ContactNameAndroid();
        mStructuredAddress = new ContactAddressAndroid();
    }

    public void dump() {
        Log.d("chromium", "===================");
        Log.d("chromium", "ContactObjectAndroid DUMP");
        Log.d("chromium", "id="+mID);
        Log.d("chromium", "name="+ mDisplayName);
        Log.d("chromium", "phone number="+mPhoneNumber);
        Log.d("chromium", "email="+mEmailAddress);
        Log.d("chromium", "address="+mAddress);
        //Log.d("chromium", "categories="+ mCategories);
        for(String category : mNewCategories) {
            Log.d("chromium", "categories="+category);
        }
        Log.d("chromium", "account name="+mAccountName);
        Log.d("chromium", "account type="+mAccountType);
        mStructuredName.dump();
        mStructuredAddress.dump();
        Log.d("chromium", "===================");
    }

    public ContactObjectAndroid copy() {
        ContactObjectAndroid newObj = new ContactObjectAndroid();

        newObj.mID = this.mID;
        newObj.mDisplayName = this.mDisplayName;
        newObj.mPhoneNumber = this.mPhoneNumber;
        newObj.mEmailAddress = this.mEmailAddress;
        newObj.mAddress = this.mAddress;
        newObj.mCategories = this.mCategories;
        newObj.mAccountName = this.mAccountName;
        newObj.mAccountType = this.mAccountType;
        newObj.mStructuredName = this.mStructuredName.copy();
        newObj.mStructuredAddress = this.mStructuredAddress.copy();

        return newObj;
    }

    public void unnullify() {
        if(TextUtils.isEmpty(mID))
            mID = " ";
        if(TextUtils.isEmpty(mDisplayName))
            mDisplayName = " ";
        if(TextUtils.isEmpty(mPhoneNumber))
            mPhoneNumber = " ";
        if(TextUtils.isEmpty(mEmailAddress))
            mEmailAddress = " ";
        if(TextUtils.isEmpty(mAddress))
            mAddress = " ";
        if(TextUtils.isEmpty(mCategories))
            mCategories = " ";
        if(TextUtils.isEmpty(mAccountName))
            mAccountName = " ";
        if(TextUtils.isEmpty(mAccountType))
            mAccountType = " ";

        mStructuredName.unnullify();
        mStructuredAddress.unnullify();
    }
}