# include <time.h>
# include <string.h>
# include <iostream>
# include <vector>

// using namespace std;

class Date{
	public:
		Date(const std::string date_str);  // constructor
		~Date();  // deconstructor
		struct tm* stringToDate(const std::string date_str);  // convert string to tm struct
		long dateToInt(struct tm *time_info);  // convert tm struct to long(time_t)
		long stringToInt(const std::string date_str);  // convert string to long(time_t)
		int parseInt(const char* str);  // parse string into int
		char* ascTimePrint(const struct tm *timeptr);  // print date
		void setDateInfo(struct tm* this_date_info);  // set date_info
		void setRawTime(long this_raw_time);  // set raw_time
		struct tm* getDateInfo();  // get date_info
		long getRawTime();  // get raw_time
	private:
		struct tm* date_info;  // date struct
		long raw_time;  // sec number
};

Date::Date(const std::string date_str){
	struct tm* this_date_info = stringToDate(date_str);
	setDateInfo(this_date_info);
	long this_raw_time = dateToInt(date_info);
	setRawTime(this_raw_time);
}

Date::~Date(){
}

struct tm* Date::stringToDate(const std::string date_str){
	std::string datestr = date_str;
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

long Date::dateToInt(struct tm *time_info){
	return (long)mktime(time_info);
}

long Date::stringToInt(const std::string date_str){
	struct tm* time_info = stringToDate(date_str);
	return (long)mktime(time_info);
}

int Date::parseInt(const char* str){
	int value = atoi(str);
	return value;
}

char* Date::ascTimePrint(const struct tm *timeptr){
	static const char mon_name[][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char result[13];
	sprintf(result, "%.3s%3d %d\n", mon_name[timeptr->tm_mon], timeptr->tm_mday, 1900 + timeptr->tm_year);
	return result;
}

void Date::setDateInfo(struct tm* this_date_info){
	date_info = this_date_info;
}

void Date::setRawTime(long this_raw_time){
	raw_time = this_raw_time;
}

struct tm* Date::getDateInfo(){
	return date_info;
}

long Date::getRawTime(){
	return raw_time;
}


int main(){
	Date my_date("2018-10-01");
	
	struct tm* my_date_info = my_date.getDateInfo();
	printf("Your input date is: %s", my_date.ascTimePrint(my_date_info));
	
	long my_raw_time = my_date.getRawTime();
	printf("Your sec number is: %d s\n", my_raw_time);
	
	struct tm* another_date_info = my_date.stringToDate("2017-01-01");
	printf("Your input date is: %s", my_date.ascTimePrint(another_date_info));
	
	long another_raw_time_1 = my_date.dateToInt(another_date_info);
	printf("Your sec number is: %d s\n", another_raw_time_1);
	
	long another_raw_time_2 = my_date.stringToInt("2017-01-01");
	printf("Your sec number is: %d s\n", another_raw_time_2);
}


