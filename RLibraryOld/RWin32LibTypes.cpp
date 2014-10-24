#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

//rtypes::rwin32 operator overloads
const Point rwin32::operator +(const Point& p1,const Point& p2)
{
    Point r = p1;
    r += p2;
    return r;
}
const Point rwin32::operator -(const Point& p1,const Point& p2)
{
    Point r = p1;
    r -= p2;
    return r;
}
const Size rwin32::operator +(const Size& s1,const Size& s2)
{
    Size r = s1;
    r += s2;
    return r;
}
const Size rwin32::operator -(const Size& s1,const Size& s2)
{
    Size r = s1;
    r -= s2;
    return r;
}

Point& Point::operator +=(const Point& p)
{
    x += p.x;
    y += p.y;
    return *this;
}
Point& Point::operator -=(const Point& p)
{
    x -= p.x;
    y -= p.y;
    return *this;
}

Size& Size::operator +=(const Size& s)
{
    width += s.width;
    height += s.height;
    return *this;
}
Size& Size::operator -=(const Size& s)
{
    width -= s.width;
    height -= s.height;
    return *this;
}

bool Rect::overlaps(const Point& p) const
{
    return p.x>=x && p.x<=x+width && p.y>=y && p.y<=y+height;
}
bool Rect::overlaps(const Rect& r) const
{
    static bool chk = false; // allows our recursion to stop
    if (r.overlaps((Point)*this) || r.overlaps(Point(x+width,y)) || r.overlaps(Point(x,y+height))
        || r.overlaps(Point(x+width,y+height)))
    {
        chk = false;
        return true;
    }
    else if (!chk)
    {// 'r' might be bigger than our rectangle and encompass it
        chk = true;
        return r.overlaps(*this);
    }
    else
    {
        chk = false;
        return false;
    }
}
Rect& Rect::operator +=(const Point& p)
{
    Point* ptr = this;
    *ptr += p;
    return *this;
}
Rect& Rect::operator -=(const Point& p)
{
    Point* ptr = this;
    *ptr -= p;
    return *this;
}
Rect& Rect::operator +=(const Size& s)
{
    Size* ptr = this;
    *ptr += s;
    return *this;
}
Rect& Rect::operator -=(const Size& s)
{
    Size* ptr = this;
    *ptr -= s;
    return *this;
}

str SystemTime::MonthToString(SystemTime::Month m)
{
    switch (m)
    {
    case January:
        return "January";
    case February:
        return "February";
    case March:
        return "March";
    case April:
        return "April";
    case May:
        return "May";
    case June:
        return "June";
    case July:
        return "July";
    case August:
        return "August";
    case September:
        return "September";
    case October:
        return "October";
    case November:
        return "November";
    case December:
        return "December";
    }
    return "BADMONTH";
}
str SystemTime::DayOfWeekToString(SystemTime::DayOfWeek dow)
{
    switch (dow)
    {
    case Sunday:
        return "Sunday";
    case Monday:
        return "Monday";
    case Tuesday:
        return "Tuesday";
    case Wednesday:
        return "Wednesday";
    case Thursday:
        return "Thursday";
    case Friday:
        return "Friday";
    case Saturday:
        return "Saturday";
    }
    return "BADDAYOFWEEK";
}
SystemTime SystemTime::GetCurrentSystemTime()
{// this time is expressed in Coordinated Universal Time
    SYSTEMTIME time;
    ::GetSystemTime(&time);
    return SystemTime(&time);
}
SystemTime SystemTime::GetCurrentLocalTime()
{// local machine time
    SYSTEMTIME time;
    ::GetLocalTime(&time);
    return SystemTime(&time);
}

SystemTime::SystemTime(PSYSTEMTIME p)
{
    *this = *p;
}
SystemTime::SystemTime(const SYSTEMTIME &st)
{
    year = st.wYear;
    month = (Month) st.wMonth;
    day = st.wDay;
    dayOfWeek = (DayOfWeek) st.wDayOfWeek;
    hour = st.wHour;
    minute = st.wMinute;
    second = st.wSecond;
    milliseconds = st.wMilliseconds;
}
SystemTime& SystemTime::operator =(PSYSTEMTIME p)
{
    *this = *p;
    return *this;
}
SystemTime& SystemTime::operator =(const SYSTEMTIME& st)
{
    year = st.wYear;
    month = (Month) st.wMonth;
    day = st.wDay;
    dayOfWeek = (DayOfWeek) st.wDayOfWeek;
    hour = st.wHour;
    minute = st.wMinute;
    second = st.wSecond;
    milliseconds = st.wMilliseconds;
    return *this;
}
str SystemTime::AsString() const
{
    str r;
    rstringstream ss(r);
    ss << (hour<10 ? "0" : "") << hour << ':' << (minute<10 ? "0" : "") << minute << ':'
        << (second<10 ? "0" : "") << second;
    ss.put(' ');
    ss << DayOfWeekToString(dayOfWeek) << ", "
        << MonthToString(month) << ' ' << day << ", "
        << year;
    return r;
}