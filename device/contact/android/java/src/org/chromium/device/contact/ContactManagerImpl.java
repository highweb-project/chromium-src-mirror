// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.contact;

import java.util.ArrayList;

import android.content.Context;
import android.util.Log;

import org.chromium.mojo.system.MojoException;
import org.chromium.mojom.device.ContactAddress;
import org.chromium.mojom.device.ContactManager;
import org.chromium.mojom.device.ContactName;
import org.chromium.mojom.device.ContactObject;

import org.chromium.device.contact.ContactConstantsAndroid.*;
import org.chromium.device.contact.ContactHandlerAndroid;
import org.chromium.device.contact.OnContactResultListener;
import org.chromium.device.contact.object.ContactObjectAndroid;
import org.chromium.device.contact.object.ContactAddressAndroid;
import org.chromium.device.contact.object.ContactNameAndroid;

public class ContactManagerImpl implements ContactManager, ContactConstantsAndroid, OnContactResultListener {
    private static final String TAG = "ContactManagerImpl";
    
    private Context mContext;
    private ContactHandlerAndroid mContactHandler;
    
    public ContactManagerImpl(Context context) {
        mContext = context;
        mContactHandler = new ContactHandlerAndroid(context);
    }

    @Override
    public void close() {}

    @Override
    public void onConnectionError(MojoException e) {}

    @Override
    public void findContact(int requestId, int target, int maxItem, String condition, FindContactResponse callback) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = ContactOperations.FIND;
    	holder.mRequestId = requestId;
    	holder.mFindCallback = callback;
    	
    	mContactHandler.doFindContact(target, maxItem, condition, this, holder);
    }
    
    @Override
    public void addContact(int requestId, ContactObject contact, AddContactResponse callback) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = ContactOperations.ADD;
    	holder.mRequestId = requestId;
    	holder.mAddCallback = callback;
    	
    	mContactHandler.doAddContact(convertToAndroidContact(contact), this, holder);
    }
    
    @Override
    public void deleteContact(int requestId, int target, int maxItem, String condition, DeleteContactResponse callback) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = ContactOperations.DELETE;
    	holder.mRequestId = requestId;
    	holder.mDeleteCallback = callback;
    	
    	mContactHandler.doDeleteContact(target, maxItem, condition, this, holder);
    }
    
    @Override
    public void updateContact(int requestId, ContactObject contact, UpdateContactResponse callback) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = ContactOperations.UPDATE;
    	holder.mRequestId = requestId;
    	holder.mUpdateCallback = callback;
    	
    	mContactHandler.doUpdateContact(convertToAndroidContact(contact), this, holder);
    }
    
    @Override
    public void onWorkFinished(ContactConstantsAndroid.Error error, ArrayList<ContactObjectAndroid> results, Object callerData) {
    	Log.d("chromium", "onWorkFinished, error="+error);
    	
    	NativeRequestHolder holder = (NativeRequestHolder)callerData;
    	
    	ContactObject[] nativeResults = null;
    	if(error == Error.SUCCESS) {
    		nativeResults = new ContactObject[results.size()];
    		for(int i=0; i<results.size(); i++)
    			nativeResults[i] = convertToNativeContact(results.get(i));
    	}
    	
    	switch(holder.mOperationType) {
    	case FIND:
    		holder.mFindCallback.call(holder.mRequestId, error.getValue(), nativeResults);
    		break;
    	case ADD:
    		holder.mAddCallback.call(holder.mRequestId, error.getValue(), nativeResults);
    		break;
    	case UPDATE:
    		holder.mUpdateCallback.call(holder.mRequestId, error.getValue(), nativeResults);
    		break;
    	case DELETE:
    		holder.mDeleteCallback.call(holder.mRequestId, error.getValue(), nativeResults);
    		break;
    	}
    }
    
    private ContactObjectAndroid convertToAndroidContact(ContactObject mojoContact) {
    	ContactObjectAndroid androidContact = new ContactObjectAndroid();
    	
    	androidContact.mID = mojoContact.id;
    	androidContact.mDisplayName = mojoContact.displayName;
    	androidContact.mPhoneNumber = mojoContact.phoneNumber;
    	androidContact.mEmailAddress = mojoContact.emails;
    	androidContact.mAddress = mojoContact.address;
    	androidContact.mAccountName = mojoContact.accountName;
    	androidContact.mAccountType = mojoContact.accountType;
    	if(mojoContact.categories != null) {
	    	for(String category : mojoContact.categories) {
	    		androidContact.mNewCategories.add(category);
	    	}
    	}
    	
    	androidContact.mStructuredAddress.mType = mojoContact.structuredAddress.type;
    	androidContact.mStructuredAddress.mStreetAddress = mojoContact.structuredAddress.streetAddress;
    	androidContact.mStructuredAddress.mLocality = mojoContact.structuredAddress.locality;
    	androidContact.mStructuredAddress.mRegion = mojoContact.structuredAddress.region;
    	androidContact.mStructuredAddress.mPostalCode = mojoContact.structuredAddress.postalCode;
    	androidContact.mStructuredAddress.mCountry = mojoContact.structuredAddress.country;
    	
    	androidContact.mStructuredName.mFamilyName = mojoContact.structuredName.familyName;
    	androidContact.mStructuredName.mGivenName = mojoContact.structuredName.givenName;
    	androidContact.mStructuredName.mMiddleName = mojoContact.structuredName.middleName;
    	androidContact.mStructuredName.mHonorificPrefix = mojoContact.structuredName.honorificPrefix;
    	androidContact.mStructuredName.mHonorificSuffix = mojoContact.structuredName.honorificSuffix;
    	
    	return androidContact;
    }
    
    private ContactObject convertToNativeContact(ContactObjectAndroid androidContact) {
    	androidContact.unnullify();
    	
    	androidContact.dump();
    	
    	ContactObject nativeContact = new ContactObject();
    	
    	nativeContact.structuredName = new ContactName();
    	nativeContact.structuredAddress = new ContactAddress();
    	
    	nativeContact.id = androidContact.mID;
    	nativeContact.displayName = androidContact.mDisplayName;
    	nativeContact.phoneNumber = androidContact.mPhoneNumber;
    	nativeContact.emails = androidContact.mEmailAddress;
    	nativeContact.address = androidContact.mAddress;
    	if(androidContact.mNewCategories.size() > 0)
    		nativeContact.categories = androidContact.mNewCategories.toArray(new String[androidContact.mNewCategories.size()]);
    	nativeContact.accountName = androidContact.mAccountName;
    	nativeContact.accountType = androidContact.mAccountType;
    	
    	nativeContact.structuredAddress.type = androidContact.mStructuredAddress.mType;
    	nativeContact.structuredAddress.streetAddress = androidContact.mStructuredAddress.mStreetAddress;
    	nativeContact.structuredAddress.locality = androidContact.mStructuredAddress.mLocality;
    	nativeContact.structuredAddress.region = androidContact.mStructuredAddress.mRegion;
    	nativeContact.structuredAddress.postalCode = androidContact.mStructuredAddress.mPostalCode;
    	nativeContact.structuredAddress.country = androidContact.mStructuredAddress.mCountry;
    	
    	nativeContact.structuredName.familyName = androidContact.mStructuredName.mFamilyName;
    	nativeContact.structuredName.givenName = androidContact.mStructuredName.mGivenName;
    	nativeContact.structuredName.middleName = androidContact.mStructuredName.mMiddleName;
    	nativeContact.structuredName.honorificPrefix = androidContact.mStructuredName.mHonorificPrefix;
    	nativeContact.structuredName.honorificSuffix = androidContact.mStructuredName.mHonorificSuffix;
    	
    	return nativeContact;
    }
    
    private class NativeRequestHolder {
    	public ContactOperations mOperationType;
    	public int mRequestId;
    	public FindContactResponse mFindCallback;
    	public AddContactResponse mAddCallback;
    	public DeleteContactResponse mDeleteCallback;
    	public UpdateContactResponse mUpdateCallback;
    }
}
