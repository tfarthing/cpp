#include <cpp/data/Integer.h>
#include <cpp/time/Duration.h>

namespace cpp
{

    const Duration Duration::Zero{ 0 };
    const Duration Duration::Infinite{ INT64_MAX };

    cpp::String Duration::toString( ) const
	{
		String result;

		auto ndays = days();
		auto nhours = hours() % 24;
		auto nminutes = minutes() % 60;
		auto nseconds = seconds() % 60;
		auto nmillis = millis() % 1000;
		auto nmicros = micros() % 1000;

		if (ndays != 0)
		{
			result += format("% day", ndays);
            if ( ndays > 1)
                { text += "s"; }
			if (!nhours && !nminutes && !nseconds && !nmillis && !nmicros)
				{ return text; }
			text += ", ";
		}
		if (nhours != 0)
		{
			text += format("% hour", nhours);
            if ( nhours > 1)
                { text += "s"; }
			if (!nminutes && !nseconds && !nmillis && !nmicros)
				{ return text; }
			text += ", ";
		}
		if (nminutes != 0)
		{
			text += String::format("% minute", nminutes);
            if ( nminutes > 1)
                { text += "s"; }
			if (!nseconds && !nmillis && !nmicros)
				{ return text; }
			text += ", ";
		}

		text += Integer::toDecimal(nseconds, text.isEmpty() ? 1 : 2, true);
		if (nmillis || nmicros)
		{
			text += "." + Integer::toDecimal(nmillis, 3, 3, true);
			if (nmicros)
				{ text += Integer::toDecimal(nmicros, 3, 3, true); }
		}
        text += " second";
        if ( nseconds != 1 || nmillis || nmicros )
            { text += "s"; }
        return text;
	}

}