# include <time.h>
# include <string.h>
# include <iostream>
# include <vector>

// using namespace std;

class MDB_DATE
{
	private:
		struct tm* date_info;  // date struct
		long raw_time;  // sec number
		char* date_str;  // date string

	public:
		// constructor
		MDB_DATE(const std::string this_date_str)
		{
			char * this_date_char = new char[20];
			strcpy(this_date_char, this_date_str.c_str());
			setDateStr(this_date_char);
			struct tm* this_date_info = stringToDate(this_date_str);
			setDateInfo(this_date_info);
			long this_raw_time = dateToLong(this_date_info);
			setRawTime(this_raw_time);
		}

		// deconstructor
		~MDB_DATE()
		{
		}

		// convert string to tm struct
		struct tm* stringToDate(const std::string this_date_str)
		{
			std::string datestr = this_date_str;
			std::vector<int> yyyymmdd(3, 0);  // int vector to save data

			// find index by splitstr namely "-"
			std::string splitstr = "-";
			std::size_t first = datestr.find(splitstr);
			std::size_t last = datestr.find_last_of(splitstr);

			time_t rawtime;
			struct tm * timeinfo;

			time (&rawtime);  // current time
			timeinfo = localtime(&rawtime);  // time_t to tm

			// parse string into int type
			yyyymmdd[0] = parseInt(datestr.substr(0, first).c_str());
			yyyymmdd[1] = parseInt(datestr.substr(first + 1, last - first - 1).c_str());
			yyyymmdd[2] = parseInt(datestr.substr(last + 1, datestr.length()).c_str());

			// save int data into int vector
			timeinfo->tm_year = yyyymmdd[0] - 1900;
			timeinfo->tm_mon = yyyymmdd[1] - 1;
			timeinfo->tm_mday = yyyymmdd[2];

			return timeinfo;
		}

		// convert tm struct to long(time_t)
		long dateToLong(struct tm *time_info)
		{
			return (long)mktime(time_info);
		}

		// convert string to long(time_t)
		long stringToLong(const std::string date_str)
		{
			struct tm* time_info = stringToDate(date_str);
			return (long)mktime(time_info);
		}

		// convert long to date
		struct tm * longToDate(long time)
		{
			time_t sec_time = (time_t)time;
			struct tm * time_info = localtime(&sec_time);
			return time_info;
		}

		// convert date to string
		char* dateToString(struct tm *time_info)
		{
			char* date_string = new char[20];
			strftime(date_string, 20, "%Y-%m-%d", time_info);
			return date_string;
		}

		// convert long to string
		char* longToString(long time)
		{
			char* date_string = new char[20];
			struct tm * time_info = longToDate(time);
			date_string = dateToString(time_info);
			return date_string;
		}

		// parse string into int
		int parseInt(const char* str)
		{
			int value = atoi(str);
			return value;
		}

		// print date
		char* ascTimePrint(const struct tm *timeptr)
		{
			static const char mon_name[][4] = {
				"Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
			};
			static char result[13];
			sprintf(result, "%.3s%3d %d\n", mon_name[timeptr->tm_mon], timeptr->tm_mday, 1900 + timeptr->tm_year);
			return result;
		}

		// set date_info
		void setDateInfo(struct tm* this_date_info)
		{
			date_info = this_date_info;
		}

		// set raw_time
		void setRawTime(long this_raw_time)
		{
			raw_time = this_raw_time;
		}

		// set Date Str
		void setDateStr(char * this_date_str)
		{
			date_str = this_date_str;
		}

		// get date_info
		struct tm* getDateInfo()
		{
			return date_info;
		}

		// get raw_time
		long getRawTime()
		{
			return raw_time;
		}

		// get date_str
		char * getDateStr()
		{
			return date_str;
		}
};


int main(){
	MDB_DATE my_date("2018-10-01");

	printf("Get information...\n");

	// get Date Info (Date)
	struct tm* my_date_info = my_date.getDateInfo();
	printf("Your input date is: %s", my_date.ascTimePrint(my_date_info));

	// get Raw Time (Long)
	long my_raw_time = my_date.getRawTime();
	printf("Your sec number is: %d s\n", my_raw_time);

	// get Date Str (String)
	char * my_date_str = my_date.getDateStr();
	printf("Your input date string is: %s\n", my_date_str);

	printf("\nConvert String to Long...\n");

	// convert String to Date
	char * another_date_str = "2017-01-01";
	struct tm* another_date_info = my_date.stringToDate(another_date_str);
	printf("Your input date is: %s", my_date.ascTimePrint(another_date_info));

	// convert Date to Long
	long another_raw_time_1 = my_date.dateToLong(another_date_info);
	printf("Your sec number is: %d s\n", another_raw_time_1);

	// convert String to Long
	long another_raw_time_2 = my_date.stringToLong(another_date_str);
	printf("Your sec number is: %d s\n", another_raw_time_2);

	printf("\nConvert Long to String...\n");

	// convert Long to Date
	long the_raw_time = 1483275979;
	struct tm* the_date_info = my_date.longToDate(the_raw_time);
	printf("Your input date is: %s", my_date.ascTimePrint(the_date_info));

	// conver Date to String
	char * the_date_str_1 = my_date.dateToString(the_date_info);
	printf("Your date string is: %s \n", the_date_str_1);

	// convert Long to String
	char * the_date_str_2 = my_date.longToString(the_raw_time);
	printf("Your date string is: %s \n", the_date_str_2);
}

