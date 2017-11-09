package org.chromium.device.contact.object;

import android.text.TextUtils;
import android.util.Log;

/**
 * Created by azureskybox on 15. 12. 9.
 */
public class ContactNameAndroid {

    public String mFamilyName;
    public String mGivenName;
    public String mMiddleName;
    public String mHonorificPrefix;
    public String mHonorificSuffix;

    public ContactNameAndroid() {
        mFamilyName = "";
        mGivenName = "";
        mMiddleName = "";
        mHonorificPrefix = "";
        mHonorificSuffix = "";
    }

    public void dump() {
        Log.d("chromium", "===================");
        Log.d("chromium", "ContactNameAndroid DUMP");
        Log.d("chromium", "family name="+mFamilyName);
        Log.d("chromium", "given name="+mGivenName);
        Log.d("chromium", "middle name="+mMiddleName);
        Log.d("chromium", "honorific prefix="+mHonorificPrefix);
        Log.d("chromium", "honorific suffix="+mHonorificSuffix);
        Log.d("chromium", "===================");
    }

    public ContactNameAndroid copy() {
        ContactNameAndroid newObj = new ContactNameAndroid();

        newObj.mFamilyName = this.mFamilyName;
        newObj.mGivenName = this.mGivenName;
        newObj.mMiddleName = this.mMiddleName;
        newObj.mHonorificPrefix = this.mHonorificPrefix;
        newObj.mHonorificSuffix = this.mHonorificSuffix;

        return newObj;
    }

    public void unnullify() {
        if(TextUtils.isEmpty(mFamilyName))
            mFamilyName = " ";
        if(TextUtils.isEmpty(mGivenName))
            mGivenName = " ";
        if(TextUtils.isEmpty(mMiddleName))
            mMiddleName = " ";
        if(TextUtils.isEmpty(mHonorificPrefix))
            mHonorificPrefix = " ";
        if(TextUtils.isEmpty(mHonorificSuffix))
            mHonorificSuffix = " ";
    }

    public boolean isEmpty() {
        if(TextUtils.isEmpty(mGivenName)
                && TextUtils.isEmpty(mMiddleName)
                && TextUtils.isEmpty(mFamilyName))
            return true;
        else
            return false;
    }

}
