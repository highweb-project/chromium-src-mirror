package org.chromium.device.contact;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import org.chromium.device.contact.object.ContactFindOptionsAndroid;
import org.chromium.device.contact.object.ContactObjectAndroid;
import org.chromium.device.contact.worker.AbstractContactWorker;
import org.chromium.device.contact.worker.AddContactWorker;
import org.chromium.device.contact.worker.DeleteContactWorker;
import org.chromium.device.contact.worker.FindContactWorker;
import org.chromium.device.contact.worker.UpdateContactWorker;
import org.w3c.dom.Text;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public class ContactHandlerAndroid implements ContactConstantsAndroid, OnContactWorkerListener {

    Context mContext;

    Object mLock;

    //For Request Scheduling
    private LinkedBlockingQueue<ContactRequestHolder> mRequestQueue = null;
    private HashMap<ContactOperations, AbstractContactWorker> mWorkerMap;
    public ContactHandlerAndroid(Context context) {
        mContext = context;
        mLock = new Object();
        mRequestQueue = new LinkedBlockingQueue<>();
        mWorkerMap = new HashMap<>();
    }

    public void doFindContact(int findTarget, int maxItem, String condition, OnContactResultListener listener, Object callerData) {
        Log.d(TAG, "doFindContact");

        if(listener == null)
            return;

        ContactFindTarget target = ContactFindTarget.toEnum(findTarget);

        Log.d(TAG, "doFindContact, find target="+target+", maxItem="+maxItem+", condition="+condition);

        if(target == ContactFindTarget.NONE) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, null);
            return;
        }

        //Init ContactFindOptionsAndroid
        ContactFindOptionsAndroid findOption = new ContactFindOptionsAndroid();

        findOption.mTarget = target;

        if(maxItem == 0 || maxItem > MAX_FIND_RESULT) {
            maxItem = MAX_FIND_RESULT;
        }
        else if(maxItem < 0) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, callerData);
            return;
        }
        findOption.mMaxItem = maxItem;

        if(TextUtils.isEmpty(condition)) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, callerData);
            return;
        }
        switch(findOption.mTarget) {
            case NAME:
                findOption.mSearchConditions.mDisplayName = condition;
                break;
            case PHONE_NUMBER:
                findOption.mSearchConditions.mPhoneNumber = condition;
                break;
            case EMAIL_ADDRESS:
                findOption.mSearchConditions.mEmailAddress = condition;
                break;
        }

        //Init ContactRequestHolder for queueing current request
        ContactRequestHolder requestHolder = new ContactRequestHolder();
        requestHolder.mOperation = ContactOperations.FIND;
        requestHolder.mFindOption = findOption;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    public void doAddContact(ContactObjectAndroid contact, OnContactResultListener listener, Object callerData) {
        Log.d(TAG, "doAddContact");

        if(listener == null)
            return;

        if((contact.mStructuredName.isEmpty() && TextUtils.isEmpty(contact.mDisplayName))
                && TextUtils.isEmpty(contact.mPhoneNumber)) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, callerData);
        }

        //Init ContactRequestHolder for queueing current request
        ContactRequestHolder requestHolder = new ContactRequestHolder();
        requestHolder.mOperation = ContactOperations.ADD;
        requestHolder.mContact = contact;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    public void doUpdateContact(ContactObjectAndroid data, OnContactResultListener listener, Object callerData) {
        Log.d(TAG, "doUpdateContact");

        if(listener == null)
            return;

        if(TextUtils.isEmpty(data.mID)) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, callerData);
            return;
        }

        //Init ContactRequestHolder for queueing current request
        ContactRequestHolder requestHolder = new ContactRequestHolder();
        requestHolder.mOperation = ContactOperations.UPDATE;
        requestHolder.mContact = data;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    public void doDeleteContact(int findTarget, int maxItem, String condition, OnContactResultListener listener, Object callerData) {
        Log.d(TAG, "doFindContact");

        if(listener == null)
            return;

        ContactFindTarget target = ContactFindTarget.toEnum(findTarget);

        Log.d(TAG, "doDeleteContact, find target="+target+", maxItem="+maxItem+", condition="+condition);

        if(target == ContactFindTarget.NONE) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, null);
            return;
        }

        //Init ContactFindOptionsAndroid
        ContactFindOptionsAndroid findOption = new ContactFindOptionsAndroid();

        findOption.mTarget = target;

        if(maxItem == 0 || maxItem > MAX_FIND_RESULT) {
            maxItem = MAX_FIND_RESULT;
        }
        else if(maxItem < 0) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, callerData);
            return;
        }
        findOption.mMaxItem = maxItem;

        if(TextUtils.isEmpty(condition)) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, callerData);
            return;
        }
        switch(findOption.mTarget) {
            case NAME:
                findOption.mSearchConditions.mDisplayName = condition;
                break;
            case PHONE_NUMBER:
                findOption.mSearchConditions.mPhoneNumber = condition;
                break;
            case EMAIL_ADDRESS:
                findOption.mSearchConditions.mEmailAddress = condition;
                break;
        }

        //Init ContactRequestHolder for queueing current request
        ContactRequestHolder requestHolder = new ContactRequestHolder();
        requestHolder.mOperation = ContactOperations.DELETE;
        requestHolder.mFindOption = findOption;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    public void doDeleteContact(ContactFindOptionsAndroid deleteOptions, OnContactResultListener listener, Object callerData) {
        Log.d(TAG, "doDeleteContact");

        if(listener == null)
            return;

        //Init ContactRequestHolder for queueing current request
        ContactRequestHolder requestHolder = new ContactRequestHolder();
        requestHolder.mOperation = ContactOperations.DELETE;
        requestHolder.mFindOption = deleteOptions;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    private void startHandleRequest() {
        synchronized (mLock) {
            if (mRequestQueue.size() < 1)
                return;

            //ContactRequestHolder holder = mRequestQueue.peek();
            ContactRequestHolder holder = pushOrPopRequest(null, false);

            if(!holder.isHandling)
                holder.isHandling = true;
            else
                return;

            AbstractContactWorker worker = null;
            worker = mWorkerMap.get(holder.mOperation);
            if (worker == null) {
                switch (holder.mOperation) {
                    case FIND:
                        worker = new FindContactWorker(mContext, this);
                        break;
                    case ADD:
                        worker = new AddContactWorker(mContext, this);
                        break;
                    case UPDATE:
                        worker = new UpdateContactWorker(mContext, holder.mContact, this);
                        break;
                    case DELETE:
                        worker = new DeleteContactWorker(mContext, this);
                        break;
                }

                mWorkerMap.put(holder.mOperation, worker);
            }

            if (holder.mOperation == ContactOperations.ADD
                    || holder.mOperation == ContactOperations.UPDATE) {
                worker.setContactData(holder.mContact);
            } else {
                worker.setContactFindOptions(holder.mFindOption);
            }

            Thread t = new Thread(worker);
            t.start();
        }
    }

    private ContactRequestHolder pushOrPopRequest(ContactRequestHolder request) {
        return pushOrPopRequest(request, false);
    }

    private ContactRequestHolder pushOrPopRequest(ContactRequestHolder request, boolean delete) {
        synchronized (mLock) {
            //Pop
            if (request == null) {
                Log.e(TAG, "pop item");

                if(delete)
                    return mRequestQueue.poll();
                else
                    return mRequestQueue.peek();
            }
            //Push
            else {
                Log.e(TAG, "push item");

                mRequestQueue.add(request);

                return null;
            }
        }
    }

    @Override
    public void onWorkFinished(ContactConstantsAndroid.Error error, ArrayList<ContactObjectAndroid> results) {
        Log.e(TAG, "worker finish its working, then it try to get more works");

        ContactRequestHolder holder = pushOrPopRequest(null, true);
        if(holder != null) {
            holder.mListener.onWorkFinished(error, results, holder.mCallerData);
            startHandleRequest();
        }
        else
            Log.e(TAG, "All Workers finish its works!! Zzz...");
    }

    private class ContactRequestHolder {
        public ContactOperations mOperation;
        public ContactObjectAndroid mContact;
        public ContactFindOptionsAndroid mFindOption;
        public OnContactResultListener mListener;
        public Object mCallerData;
        public boolean isHandling = false;
    }
}
