/*
 * Copyright (c) 2024, Muhammad Waleed Badar
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtc_ds3231.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/util.h>
#include <time.h>

static struct device *const rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
static const char *rtc_msg_time;

/* Format times as: YYYY-MM-DD HH:MM:SS DOW DOY */
static const char *format_time(const struct rtc_time *rtc_tm)
{
    static char buf[RTC_MSG_BUFFER_SIZE];
    char *bp = buf;
    char const *const bpe = bp + sizeof(buf);
    
    // Convert to standard tm structure for strftime
    struct tm tm = {0};
    tm.tm_sec = rtc_tm->tm_sec;
    tm.tm_min = rtc_tm->tm_min;
    tm.tm_hour = rtc_tm->tm_hour;
    tm.tm_mday = rtc_tm->tm_mday;
    tm.tm_mon = rtc_tm->tm_mon;
    tm.tm_year = rtc_tm->tm_year;
    tm.tm_wday = rtc_tm->tm_wday;
    tm.tm_yday = rtc_tm->tm_yday;
    tm.tm_isdst = rtc_tm->tm_isdst;
    
    bp += strftime(bp, bpe - bp, "%Y-%m-%d %H:%M:%S", &tm);
    
    // Add nanoseconds if available
    if (rtc_tm->tm_nsec > 0) {
        bp += snprintf(bp, bpe - bp, ".%09d", rtc_tm->tm_nsec);
    }
    
    // Add day of week and day of year if known
    if (tm.tm_wday >= 0 && tm.tm_yday >= 0) {
        bp += strftime(bp, bpe - bp, " %a %j", &tm);
    } else if (tm.tm_wday >= 0) {
        bp += strftime(bp, bpe - bp, " %a", &tm);
    }
    
    return buf;
}

static int rtc_set_date_time()
{
	int ret = 0;
	struct rtc_time tm = {
		.tm_year = 2025 - 1900,
		.tm_mon = 9 - 1,
		.tm_mday = 21,
		.tm_hour = 4,
		.tm_min = 19,
		.tm_sec = 0,
	};

	ret = rtc_set_time(rtc, &tm);
	if (ret < 0) {
		printk("Cannot write date time: %d\n", ret);
		return ret;
	}
	return ret;
}

const char* rtc_ds3231_get_last_time(void)
{
	int ret = 0;
    struct rtc_time tm;

	ret = rtc_get_time(rtc, &tm);
	if (ret < 0) {
		printk("Cannot read date time: %d\n", ret);
		return ret;
	}

    rtc_msg_time = format_time(&tm);

    printk("RTC date and time: %s\n", rtc_msg_time);

	return rtc_msg_time;
}

void rtc_ds3231_init(void)
{
	/* Check if the RTC is ready */
	if (!device_is_ready(rtc)) {
		printk("Device is not ready\n");
        return;
	}

    rtc_set_date_time();
}
