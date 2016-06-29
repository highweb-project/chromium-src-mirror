package org.chromium.device.messaging;

import android.content.Context;
import android.content.IntentFilter;
import android.text.TextUtils;
import android.util.Log;

import org.chromium.device.messaging.object.MessageFindOptionsAndroid;
import org.chromium.device.messaging.object.MessageObjectAndroid;
import org.chromium.device.messaging.worker.AbstractMessagingWorker;
import org.chromium.device.messaging.worker.FindMessageWorker;
import org.chromium.device.messaging.worker.SendMessageWorker;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class MessagingHandlerAndroid implements MessagingConstantsAndroid, OnMessagingWorkerListener {

    Context mContext;
    Object mLock;

    //For Request Scheduling
    private LinkedBlockingQueue<MessagingRequestHolder> mRequestQueue = null;
    private HashMap<MessagingOperations, AbstractMessagingWorker> mWorkerMap;

    private boolean mIsObserverInitialized = false;
    public static MessagingObserverAndroid mObserver = null;

    private static MessagingHandlerAndroid mInstance = null;

    public static MessagingHandlerAndroid getInstance(Context context) {
        if(mInstance == null) {
            mInstance = new MessagingHandlerAndroid(context);
        }
        return mInstance;
    }

    private MessagingHandlerAndroid(Context context) {
        Log.d(TAG, "MessagingHandlerAndroid");

        mContext = context;
        mLock = new Object();
        mRequestQueue = new LinkedBlockingQueue<>();
        mWorkerMap = new HashMap<>();
    }

    public void doFindMessage(int findTarget, int maxItem, String condition, OnMessagingResultListener listener, Object callerData) {
        Log.d(TAG, "doFindMessage");

        if(listener == null)
            return;

        MessageFindTarget target = MessageFindTarget.toEnum(findTarget);

        Log.d(TAG, ">> find target="+target+", maxItem="+maxItem+", condition="+condition);

        if(target == MessageFindTarget.NONE) {
            listener.onWorkFinished(Error.INVALID_ARGUMENT_ERROR, null, null);
            return;
        }

        //Init ContactFindOptionsAndroid
        MessageFindOptionsAndroid findOption = new MessageFindOptionsAndroid();

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

        findOption.mCondition = condition;

        MessagingRequestHolder requestHolder = new MessagingRequestHolder();
        requestHolder.mOperation = MessagingOperations.FIND;
        requestHolder.mFindOption = findOption;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder, false);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    public void doSendMessage(MessageObjectAndroid message, OnMessagingResultListener listener, Object callerData) {
        Log.d(TAG, "doSendMessage");

        if(listener == null)
            return;

        MessagingRequestHolder requestHolder = new MessagingRequestHolder();
        requestHolder.mOperation = MessagingOperations.SEND;
        requestHolder.mMessage = message;
        requestHolder.mListener = listener;
        requestHolder.mCallerData = callerData;

        pushOrPopRequest(requestHolder, false);

        if(mRequestQueue.size() == 1)
            startHandleRequest();
    }

    public void doAddListener(int listenerId, OnMessagingResultListener listener, Object callerData) {
        Log.d(TAG, "doAddListener");

        if(!mIsObserverInitialized) {
            mIsObserverInitialized = true;

            if(mObserver != null) {
                mContext.unregisterReceiver(mObserver);
                mObserver = null;
            }

            mObserver = new MessagingObserverAndroid();

            mContext.registerReceiver(mObserver, new IntentFilter(MessagingObserverAndroid.ACTION));
        }

        mObserver.addListener(listenerId, listener, callerData);
    }

    public void doRemoveListener(int listenerId) {
        Log.d(TAG, "doRemoveListener");

        if(mObserver != null) {
            if(mObserver.removeListener(listenerId)) {
                Log.d(TAG, "observer doesn't have any listener, so unregister it");

                mContext.unregisterReceiver(mObserver);
                mObserver = null;

                mIsObserverInitialized = false;
            }
        }
    }

    private void startHandleRequest() {
        synchronized (mLock) {
            if (mRequestQueue.size() < 1)
                return;

            //ContactRequestHolder holder = mRequestQueue.peek();
            MessagingRequestHolder holder = pushOrPopRequest(null, false);

            if(!holder.isHandling)
                holder.isHandling = true;
            else
                return;

            AbstractMessagingWorker worker = null;
            worker = mWorkerMap.get(holder.mOperation);
            if (worker == null) {
                switch (holder.mOperation) {
                    case FIND:
                        worker = new FindMessageWorker(mContext, this);
                        break;
                    case SEND:
                        worker = new SendMessageWorker(mContext, this);
                        break;
                }

                mWorkerMap.put(holder.mOperation, worker);
            }

            if (holder.mOperation == MessagingOperations.SEND) {
                worker.setMessageData(holder.mMessage);
            } else if(holder.mOperation == MessagingOperations.FIND) {
                worker.setMessageFindOptions(holder.mFindOption);
            }

            Thread t = new Thread(worker);
            t.start();
        }
    }

    private MessagingRequestHolder pushOrPopRequest(MessagingRequestHolder request, boolean delete) {
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
    public void onWorkFinished(MessagingConstantsAndroid.Error error, ArrayList<MessageObjectAndroid> results) {
        Log.e(TAG, "worker finish its working, then it try to get more works");

        MessagingRequestHolder holder = pushOrPopRequest(null, true);
        if(holder != null) {
            holder.mListener.onWorkFinished(error, results, holder.mCallerData);
            startHandleRequest();
        }
        else
            Log.e(TAG, "All Workers finish its works!! Zzz...");
    }

    private class MessagingRequestHolder {
        public MessagingOperations mOperation;
        public MessageObjectAndroid mMessage;
        public MessageFindOptionsAndroid mFindOption;
        public OnMessagingResultListener mListener;
        public Object mCallerData;
        public boolean isHandling = false;
    }
}
