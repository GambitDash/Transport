#ifndef INCL_CTIMEDATE
#define INCL_CTIMEDATE

//	TimeDate classes

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#define SECONDS_PER_DAY					(60 * 60 * 24)

class CTimeDate;
class CTimeSpan;

class CTimeDate
	{
	public:
		enum Constants
			{
			Now,
			Today,
			};

		CTimeDate (void) { }
		CTimeDate (Constants Init);
		CTimeDate (int iDaysSince1AD, int iMillisecondsSinceMidnight);

		inline int Year (void) const { return m_Time.date().year(); }
		inline int Month (void) const { return m_Time.date().month(); }
		inline int Day (void) const { return m_Time.date().day(); }
		inline int Hour (void) const { return m_Time.time_of_day().hours(); }
		inline int Minute (void) const { return m_Time.time_of_day().minutes(); }
		inline int Second (void) const { return m_Time.time_of_day().seconds(); }
		inline int Millisecond (void) const { return m_Time.time_of_day().total_milliseconds(); }

		CString Format (const CString &sFormat) const;
		int DaysSince1AD (void) const;
		int MillisecondsSinceMidnight (void) const;

	private:
		boost::posix_time::ptime m_Time;
		friend CTimeDate operator+(const CTimeDate &iDate, const CTimeSpan &iSpan);
		friend CTimeDate operator-(const CTimeDate &iDate, const CTimeSpan &iSpan);
		friend CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime);
	};

class CTimeSpan
	{
	public:
		CTimeSpan (void);
		CTimeSpan (const CTimeSpan &);
		CTimeSpan (int iMilliseconds);
		CTimeSpan (int iDays, int iMilliseconds);

		inline int Days (void) const { return m_Span.hours() / 24; }
		inline int Minutes (void) const { return m_Span.minutes(); }
		inline int Seconds (void) const { return m_Span.seconds(); }
		inline int Milliseconds (void) const { return m_Span.total_milliseconds(); }

		CString Format (const CString &sFormat) const;
		void ReadFromStream (IReadStream *pStream);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		boost::posix_time::time_duration m_Span;
		friend CTimeDate operator+(const CTimeDate &iDate, const CTimeSpan &iSpan);
		friend CTimeDate operator-(const CTimeDate &iDate, const CTimeSpan &iSpan);
		friend CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime);
	};

CTimeDate operator+(CTimeDate &iDate, CTimeSpan &iSpan);
CTimeDate operator-(CTimeDate &iDate, CTimeSpan &iSpan);
CTimeDate timeAddTime (const CTimeDate &StartTime, const CTimeSpan &Addition);
CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime);
CTimeDate timeSubtractTime (const CTimeDate &StartTime, const CTimeSpan &Subtraction);
bool timeIsLeapYear (int iYear);


#endif

