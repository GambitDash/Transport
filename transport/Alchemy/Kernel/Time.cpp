//	Time.cpp
//
//	Time package

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"
#include "CMath.h"

#include "CTimeDate.h"
#include "CStream.h"

using namespace boost::posix_time;
using namespace boost::gregorian;


//	CTimeDate -----------------------------------------------------------------

CTimeDate::CTimeDate (Constants Init)

//	CTimeDate constructor

	{
	switch (Init)
		{
		case Now:
			m_Time = second_clock::local_time();
			break;

		//	WARNING: Not sure if this works for timezones other than GMT

		case Today:
			m_Time = ptime(second_clock::local_time().date());
			break;

		default:
			ASSERT(false);
		};
	}

CTimeDate::CTimeDate (int iDaysSince1AD, int iMillisecondsSinceMidnight)
	: m_Time(date(1, Jan, 1))

//	CTimeDate constructor

	{
	m_Time = m_Time + days(iDaysSince1AD) + millisec(iMillisecondsSinceMidnight);
	}

int CTimeDate::DaysSince1AD (void) const

//	DaysSince1AD
//
//	Returns the number of days elapsed since Jan 1, 1 AD.

	{
	return (m_Time.date() - date(1, Jan, 1)).days();
	}

CString CTimeDate::Format (const CString &sFormat) const

//	Format
//
//	Formats a timedate
//
//    %a    abbreviated weekday name (Sun)
//    %A    full weekday name (Sunday)
//    %b    abbreviated month name (Dec)
//    %B    full month name (December)
//    %c    date and time (Dec  2 06:55:15 1979)
//    %d    day of the month (02)
//    %H    hour of the 24-hour day (06)
//    %I    hour of the 12-hour day (06)
//    %j    day of the year, from 001 (335)
//    %m    month of the year, from 01 (12)
//    %M    minutes after the hour (55)
//    %p    AM/PM indicator (AM)
//    %S    seconds after the minute (15)
//    %U    Sunday week of the year, from 00 (48)
//    %w    day of the week, from 0 for Sunday (6)
//    %W    Monday week of the year, from 00 (47)
//    %x    date (Dec  2 1979)
//    %X    time (06:55:15)
//    %y    year of the century, from 00 (79)
//    %Y    year (1979)
//    %Z    time zone name, if any (EST)
//    %%    percent character %

	{
	return CString(CONSTLIT("not yet implemented"));
	}

int CTimeDate::MillisecondsSinceMidnight (void) const

//	MillisecondsSinceMidnight
//
//	Returns the number of milliseconds since midnight

	{
	return m_Time.time_of_day().total_milliseconds();
	}

//	CTimeSpan -----------------------------------------------------------------

CTimeSpan::CTimeSpan (void)

//	CTimeSpan constructor

	{
	}

CTimeSpan::CTimeSpan (const CTimeSpan &sp)
	: m_Span(sp.m_Span)
	{
	}

CTimeSpan::CTimeSpan (int iMilliseconds)

//	CTimeSpan constructor

	{
	m_Span = milliseconds(iMilliseconds);
	}

CTimeSpan::CTimeSpan (int iDays, int iMilliseconds)

//	CTimeSpan constructor

	{
	m_Span = milliseconds(iMilliseconds) + hours(24 * iDays);
	}

CString CTimeSpan::Format (const CString &sFormat) const

//	Format
//
//	Formats the time span
//
//	-			1 hour and 5 minutes
//	hh:mm:ss	04:01:10.4

	{
	return strPatternSubst(CONSTLIT("%02d:%02d:%02d.%d"), Days() * 24, Minutes(), Seconds(), Milliseconds());
	}

void CTimeSpan::ReadFromStream (IReadStream *pStream)

//	ReadFromStream
//
//	Reads from a stream
//	XXX Not compatible!

	{
	DWORD sec;
	DWORD ms;

	pStream->Read((char *)&sec, sizeof(DWORD));
	pStream->Read((char *)&ms, sizeof(DWORD));

	m_Span = milliseconds(ms) + seconds(sec);
	}

void CTimeSpan::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes out to a stream

	{
	DWORD sec = m_Span.total_seconds();
	DWORD ms = m_Span.fractional_seconds();

	pStream->Write((char *)&sec, sizeof(DWORD));
	pStream->Write((char *)&ms, sizeof(DWORD));
	}

//	Functions -----------------------------------------------------------------

CTimeDate operator+(const CTimeDate &iDate, const CTimeSpan &iSpan)
	{
	CTimeDate tmp;

	tmp.m_Time = iDate.m_Time + iSpan.m_Span;

	return tmp;
	}

CTimeDate operator-(const CTimeDate &iDate, const CTimeSpan &iSpan)
	{
	CTimeDate tmp;

	tmp.m_Time = iDate.m_Time - iSpan.m_Span;

	return tmp;
	}


CTimeDate timeAddTime (const CTimeDate &StartTime, const CTimeSpan &Addition)

//	timeAddTime
//
//	Adds a timespan to a timedate

	{
	return StartTime + Addition;
	}

CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime)

//	timeSpan
//
//	Returns the difference between the two times

	{
	CTimeSpan tmp;
	tmp.m_Span = EndTime.m_Time - StartTime.m_Time;
	return tmp;
	}

CTimeDate timeSubtractTime (const CTimeDate &StartTime, const CTimeSpan &Subtraction)

//	timeSubtractTime
//
//	Subtracts time from timedate

	{
	return StartTime - Subtraction;
	}

