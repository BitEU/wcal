#include "calendar.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

static const char *month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const char *day_names[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int get_days_in_month(int year, int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    
    return days[month - 1];
}

int get_day_of_week(int year, int month, int day) {
    // Zeller's congruence algorithm
    if (month < 3) {
        month += 12;
        year--;
    }
    
    int k = year % 100;
    int j = year / 100;
    
    int h = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    
    // Zeller's returns: Saturday=0, Sunday=1, Monday=2, Tuesday=3, Wednesday=4, Thursday=5, Friday=6
    // We need: Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6
    return (h + 6) % 7;
}

int get_first_day_of_month(int year, int month) {
    return get_day_of_week(year, month, 1);
}

int get_week_number(int year, int month, int day) {
    struct tm date = {0};
    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    
    mktime(&date);
    
    // ISO 8601 week number
    char buffer[3];
    strftime(buffer, sizeof(buffer), "%W", &date);
    
    return atoi(buffer);
}

const char* get_month_name(int month) {
    if (month >= 1 && month <= 12) {
        return month_names[month - 1];
    }
    return "Invalid";
}

const char* get_day_name(int day_of_week) {
    if (day_of_week >= 0 && day_of_week <= 6) {
        return day_names[day_of_week];
    }
    return "Invalid";
}

int compare_dates(Date d1, Date d2) {
    if (d1.year != d2.year) return d1.year - d2.year;
    if (d1.month != d2.month) return d1.month - d2.month;
    return d1.day - d2.day;
}

int compare_datetimes(DateTime dt1, DateTime dt2) {
    if (dt1.year != dt2.year) return dt1.year - dt2.year;
    if (dt1.month != dt2.month) return dt1.month - dt2.month;
    if (dt1.day != dt2.day) return dt1.day - dt2.day;
    if (dt1.hour != dt2.hour) return dt1.hour - dt2.hour;
    return dt1.minute - dt2.minute;
}

void add_days_to_date(Date *date, int days) {
    // Simple implementation - can be optimized
    while (days > 0) {
        date->day++;
        if (date->day > get_days_in_month(date->year, date->month)) {
            date->day = 1;
            date->month++;
            if (date->month > 12) {
                date->month = 1;
                date->year++;
            }
        }
        days--;
    }
    
    while (days < 0) {
        date->day--;
        if (date->day < 1) {
            date->month--;
            if (date->month < 1) {
                date->month = 12;
                date->year--;
            }
            date->day = get_days_in_month(date->year, date->month);
        }
        days++;
    }
}

void add_months_to_date(Date *date, int months) {
    date->month += months;
    
    while (date->month > 12) {
        date->month -= 12;
        date->year++;
    }
    
    while (date->month < 1) {
        date->month += 12;
        date->year--;
    }
    
    // Adjust day if necessary
    int max_days = get_days_in_month(date->year, date->month);
    if (date->day > max_days) {
        date->day = max_days;
    }
}