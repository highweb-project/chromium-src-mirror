package org.chromium.device.contact.object;

import android.text.TextUtils;
import android.util.Log;

/**
 * Created by azureskybox on 15. 12. 14.
 */
public class ContactAddressAndroid {

    public static final String DEFAULT_ADDRESS_TYPE_HOME = "Home";
    public static final String DEFAULT_ADDRESS_TYPE_WORK = "Work";
    public static final String DEFAULT_ADDRESS_TYPE_OTHER = "Other";

    public String mType;
    public String mStreetAddress;
    public String mLocality;
    public String mRegion;
    public String mPostalCode;
    public String mCountry;

    public ContactAddressAndroid() {
        mType = "";
        mStreetAddress = "";
        mLocality = "";
        mRegion = "";
        mPostalCode = "";
        mCountry = "";
    }

    public void dump() {
        Log.d("chromium", "===================");
        Log.d("chromium", "ContactAddressAndroid DUMP");
        Log.d("chromium", "type="+mType);
        Log.d("chromium", "street="+ mStreetAddress);
        Log.d("chromium", "locality="+mLocality);
        Log.d("chromium", "region="+mRegion);
        Log.d("chromium", "postal code="+mPostalCode);
        Log.d("chromium", "country=" + mCountry);
        Log.d("chromium", "===================");
    }

    public ContactAddressAndroid copy() {
        ContactAddressAndroid newObj = new ContactAddressAndroid();

        newObj.mType = this.mType;
        newObj.mStreetAddress = this.mStreetAddress;
        newObj.mLocality = this.mLocality;
        newObj.mRegion = this.mRegion;
        newObj.mPostalCode = this.mPostalCode;
        newObj.mCountry = this.mCountry;

        return newObj;
    }

    public void unnullify() {
        if(TextUtils.isEmpty(mType))
            mType = " ";
        if(TextUtils.isEmpty(mStreetAddress))
            mStreetAddress = " ";
        if(TextUtils.isEmpty(mLocality))
            mLocality = " ";
        if(TextUtils.isEmpty(mRegion))
            mRegion = " ";
        if(TextUtils.isEmpty(mPostalCode))
            mPostalCode = " ";
        if(TextUtils.isEmpty(mCountry))
            mCountry = " ";
    }

    public boolean isEmpty() {
        if(TextUtils.isEmpty(mStreetAddress)
                && TextUtils.isEmpty(mLocality)
                && TextUtils.isEmpty(mRegion)) {
            return true;
        }
        else
            return false;
    }

}
