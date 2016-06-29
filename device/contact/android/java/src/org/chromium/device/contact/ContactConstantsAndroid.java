package org.chromium.device.contact;

/**
 * Created by azureskybox on 15. 12. 8.
 */
public interface ContactConstantsAndroid {
    String TAG = "chromium";

    int MAX_FIND_RESULT = 30;

    enum ContactOperations {
        NONE(-1),
        ADD(0),
        UPDATE(1),
        DELETE(2),
        FIND(3);

        private int mValue;

        ContactOperations(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

        static ContactOperations toEnum(int value) {
            switch(value) {
                case 0:
                    return ContactOperations.ADD;
                case 1:
                    return ContactOperations.UPDATE;
                case 2:
                    return ContactOperations.DELETE;
                case 3:
                    return ContactOperations.FIND;
                default:
                    return ContactOperations.NONE;
            }
        }
    }

    enum ContactFindTarget {
        NONE(-1),
        NAME(0),
        PHONE_NUMBER(1),
        EMAIL_ADDRESS(2);

        private int mValue;

        ContactFindTarget(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

        static ContactFindTarget toEnum(int value) {
            switch(value) {
                case 0:
                    return NAME;
                case 1:
                    return PHONE_NUMBER;
                case 2:
                    return EMAIL_ADDRESS;
                default:
                    return NONE;
            }
        }
    }

    enum Error {
        UNKNOWN_ERROR(0),
        INVALID_ARGUMENT_ERROR(1),
        TIMEOUT_ERROR(3),
        PENDING_OPERATION_ERROR(4),
        IO_ERROR(5),
        NOT_SUPPORTED_ERROR(6),
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
