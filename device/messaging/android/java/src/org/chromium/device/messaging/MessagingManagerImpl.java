// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.messaging;

import java.util.ArrayList;

import android.content.Context;
import android.util.Log;

import org.chromium.mojo.system.MojoException;
import org.chromium.mojom.device.MessagingManager;
import org.chromium.mojom.device.MessageObject;

import org.chromium.device.messaging.MessagingConstantsAndroid.*;
import org.chromium.device.messaging.MessagingHandlerAndroid;
import org.chromium.device.messaging.OnMessagingResultListener;
import org.chromium.device.messaging.object.MessageObjectAndroid;

public class MessagingManagerImpl implements MessagingManager, MessagingConstantsAndroid, OnMessagingResultListener {
    private static final String TAG = "MessagingManagerImpl";
    
    private Context mContext;
    private MessagingHandlerAndroid mMessagingHandler;
    
    public MessagingManagerImpl(Context context) {
    	Log.d("chromium", "MessagingManagerImpl created");
    	
        mContext = context;
        mMessagingHandler = MessagingHandlerAndroid.getInstance(context);
    }

    @Override
    public void close() {}

    @Override
    public void onConnectionError(MojoException e) {}

    @Override
    public void sendMessage(MessageObject message) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = MessagingOperations.SEND;
    	
    	mMessagingHandler.doSendMessage(convertToAndroidMessage(message), this, holder);
    }
    
    @Override
    public void findMessage(int requestId, int target, int maxItem, String condition, FindMessageResponse callback) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = MessagingOperations.FIND;
    	holder.mRequestId = requestId;
    	holder.mFindCallback = callback;
    	
    	mMessagingHandler.doFindMessage(target, maxItem, condition, this, holder);
    }
    
    @Override
    public void addMessagingListener(int observerId, AddMessagingListenerResponse callback) {
    	NativeRequestHolder holder = new NativeRequestHolder();
    	holder.mOperationType = MessagingOperations.OBSERVE;
    	holder.mRequestId = observerId;
    	holder.mObservingCallback = callback;
    	
		mMessagingHandler.doAddListener(observerId, this, holder);
    }
    
    @Override
    public void removeMessagingListener(int observerId) {
    	mMessagingHandler.doRemoveListener(observerId);
    }

    
    @Override
    public void onWorkFinished(MessagingConstantsAndroid.Error error, ArrayList<MessageObjectAndroid> results, Object callerData) {
    	Log.d("chromium", "onWorkFinished, error="+error);
    	
    	NativeRequestHolder holder = (NativeRequestHolder)callerData;
    	
    	MessageObject[] nativeResults = null;
    	if(error == Error.SUCCESS && results != null) {
    		nativeResults = new MessageObject[results.size()];
    		for(int i=0; i<results.size(); i++)
    			nativeResults[i] = convertToNativeMessage(results.get(i));
    	}
    	
    	switch(holder.mOperationType) {
    	case FIND:
    		holder.mFindCallback.call(holder.mRequestId, error.getValue(), nativeResults);
    		break;
    	case OBSERVE:
    		holder.mObservingCallback.call(holder.mRequestId, nativeResults[0]);
    		break;
    	}
    }
    
    private MessageObjectAndroid convertToAndroidMessage(MessageObject mojoMessage) {
    	MessageObjectAndroid androidMessage = new MessageObjectAndroid();
    	
    	androidMessage.mID = mojoMessage.id;
    	androidMessage.mType = MessageType.toEnum(mojoMessage.type);
    	androidMessage.mTo = mojoMessage.to;
    	androidMessage.mFrom = mojoMessage.from;
    	androidMessage.mTitle = mojoMessage.title;
    	androidMessage.mBody = mojoMessage.body;
    	androidMessage.mDate = mojoMessage.date;
    	
    	return androidMessage;
    }
    
    private MessageObject convertToNativeMessage(MessageObjectAndroid androidMessage) {
    	androidMessage.dump();
    	
    	MessageObject nativeMessage = new MessageObject();
    	
    	nativeMessage.id = androidMessage.mID;
    	nativeMessage.type = androidMessage.mType.name();
    	nativeMessage.to = androidMessage.mTo;
    	nativeMessage.from = androidMessage.mFrom;
    	nativeMessage.title = androidMessage.mTitle;
    	nativeMessage.body = androidMessage.mBody;
    	nativeMessage.date = androidMessage.mDate;
    	
    	return nativeMessage;
    }
    
    private class NativeRequestHolder {
    	public MessagingOperations mOperationType;
    	public int mRequestId;
    	public FindMessageResponse mFindCallback;
    	public AddMessagingListenerResponse mObservingCallback;
    }
}
