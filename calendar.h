#ifndef CALENDAR_H
#define CALENDAR_H

// Date structure
typedef struct {
    int year;
    int month;
    int day;
} Date;

// DateTime structure
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
} DateTime;

// Calendar functions
int is_leap_year(int year);
int get_days_in_month(int year, int month);
int get_first_day_of_month(int year, int month);
int get_day_of_week(int year, int month, int day);
int get_week_number(int year, int month, int day);
const char* get_month_name(int month);
const char* get_day_name(int day_of_week);
int compare_dates(Date d1, Date d2);
int compare_datetimes(DateTime dt1, DateTime dt2);
void add_days_to_date(Date *date, int days);
void add_months_to_date(Date *date, int months);

#endif // CALENDAR_H