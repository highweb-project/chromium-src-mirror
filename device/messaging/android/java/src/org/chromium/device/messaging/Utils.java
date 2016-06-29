package org.chromium.device.messaging;

import java.util.Calendar;
import java.util.Date;

/**
 * Created by azureskybox on 15. 12. 21.
 */
public class Utils {

    public static String getDateString(long ms) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(new Date(ms));

        StringBuilder builder = new StringBuilder();
        builder.append(calendar.get(Calendar.YEAR));
        builder.append("-");
        builder.append(String.format("%02d", calendar.get(Calendar.MONTH)+1));
        builder.append("-");
        builder.append(String.format("%02d", calendar.get(Calendar.DATE)));
        builder.append(" ");
        builder.append(String.format("%02d", calendar.get(Calendar.HOUR)));
        builder.append(":");
        builder.append(String.format("%02d", calendar.get(Calendar.MINUTE)));

        return builder.toString();
    }

}
