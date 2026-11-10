// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 1993-2026 Shannon Smith

/*
 * Buddhist Era arithmetic. The chronology follows the Gregorian civil
 * calendar with a 543-year display offset; ICU remains the locale formatter.
 * The public private-engine signatures retain their mode argument for ABI
 * compatibility, but no unrelated calendars are dispatched in this module.
 */
#include "calendar-arithmetic.h"
#include "julian-day.h"

static gboolean buddhist_mode(CalendarCalendarMode mode)
{
    return mode == CALENDAR_CALENDAR_MODE_BUDDHIST;
}

static gint64 gregorian_year(const CalendarCalendarFields *fields)
{
    return calendar_i64_subtract_saturating(fields->year, 543);
}

static void store_buddhist_year(gint64 year, CalendarCalendarFields *fields)
{
    fields->year = calendar_i64_add_saturating(year, 543);
    fields->auxiliary = 0;
}

static gint64 buddhist_fields_to_jdn(const CalendarCalendarFields *fields)
{
    const gint64 year = gregorian_year(fields);
    if (fields->month < 1 || fields->month > 12 ||
        fields->day < 1 ||
        fields->day > calendar_gregorian_month_length(year, fields->month))
        return G_MAXINT64;
    return calendar_gregorian_to_jdn(year, fields->month, fields->day);
}

gboolean
calendar_arithmetic_fields_from_jdn(CalendarCalendarMode mode, gint64 jdn,
                                    CalendarCalendarFields *fields)
{
    gint year, month, day;
    if (fields == NULL || !buddhist_mode(mode) ||
        !calendar_gregorian_jdn_representable(jdn))
        return FALSE;

    calendar_jdn_to_gregorian(jdn, &year, &month, &day);
    *fields = (CalendarCalendarFields){0};
    fields->month = month;
    fields->day = day;
    store_buddhist_year(year, fields);
    return TRUE;
}

gint64 calendar_arithmetic_month_start(CalendarCalendarMode mode, gint64 jdn)
{
    CalendarCalendarFields fields;
    if (!calendar_arithmetic_fields_from_jdn(mode, jdn, &fields))
        return G_MAXINT64;
    fields.day = 1;
    return buddhist_fields_to_jdn(&fields);
}

gint64 calendar_arithmetic_add_months(CalendarCalendarMode mode, gint64 jdn,
                                     gint amount)
{
    CalendarCalendarFields fields;
    gint64 serial;
    gint64 year;

    if (!calendar_arithmetic_fields_from_jdn(mode, jdn, &fields))
        return G_MAXINT64;

    serial = calendar_i64_add_saturating(
        calendar_i64_add_saturating(
            calendar_i64_multiply_saturating(gregorian_year(&fields), 12),
            fields.month - 1), amount);
    year = calendar_floor_divide(serial, 12);
    fields.month = (gint)calendar_positive_modulo(serial, 12) + 1;
    store_buddhist_year(year, &fields);
    fields.day = MIN(fields.day,
                     calendar_gregorian_month_length(year, fields.month));
    return buddhist_fields_to_jdn(&fields);
}

gint64 calendar_arithmetic_add_years(CalendarCalendarMode mode, gint64 jdn,
                                    gint amount)
{
    CalendarCalendarFields fields;
    gint64 year;

    if (!calendar_arithmetic_fields_from_jdn(mode, jdn, &fields))
        return G_MAXINT64;

    year = calendar_i64_add_saturating(gregorian_year(&fields), amount);
    store_buddhist_year(year, &fields);
    fields.day = MIN(fields.day,
                     calendar_gregorian_month_length(year, fields.month));
    return buddhist_fields_to_jdn(&fields);
}
