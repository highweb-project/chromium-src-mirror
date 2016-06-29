package org.chromium.device.messaging;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public interface MessagingConstantsAndroid {
    String TAG = "chromium";

    int MAX_FIND_RESULT = 30;

    enum MessageType {
        NONE(-1),
        SMS(0),
        MMS(1),
        EMAIL(2);

        private int mValue;

        MessageType(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

        static MessageType toEnum(String value) {
            if(value.equals("sms")) {
                return SMS;
            }
            else if(value.equals("mms")) {
                return MMS;
            }
            else if(value.equals("email")) {
                return EMAIL;
            }
            else {
                return NONE;
            }
        }

        static MessageType toEnum(int value) {
            switch(value) {
                case 0:
                    return MessageType.SMS;
                case 1:
                    return MessageType.MMS;
                case 2:
                    return MessageType.EMAIL;
                default:
                    return MessageType.NONE;
            }
        }
    }

    enum MessagingOperations {
        NONE(-1),
        SEND(0),
        OBSERVE(1),
        FIND(2);

        private int mValue;

        MessagingOperations(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

        static MessagingOperations toEnum(int value) {
            switch(value) {
                case 0:
                    return MessagingOperations.SEND;
                case 1:
                    return MessagingOperations.OBSERVE;
                case 2:
                    return MessagingOperations.FIND;
                default:
                    return MessagingOperations.NONE;
            }
        }
    }

    enum MessageFindTarget {
        NONE(-1),
        FROM(0),
        BODY(1);

        private int mValue;

        MessageFindTarget(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

        static MessageFindTarget toEnum(int value) {
            switch(value) {
                case 0:
                    return FROM;
                case 1:
                    return BODY;
                default:
                    return NONE;
            }
        }
    }

    enum Error {
        UNKNOWN_ERROR(0),
        INVALID_ARGUMENT_ERROR(1),
        TIMEOUT_ERROR(2),
        PENDING_OPERATION_ERROR(3),
        IO_ERROR(4),
        NOT_SUPPORTED_ERROR(5),
        PERMISSION_DENIED_ERROR(20),
        MESSAGE_SIZE_EXCEEDED(30),
        SUCCESS(40);

        private final int mValue;

        Error(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }
}
