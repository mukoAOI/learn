#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;
using namespace std::chrono;

// 检查是否是闰年
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取月份的天数
int getDaysInMonth(int month, int year) {
    switch (month) {
        case 1: return 31;
        case 2: return isLeapYear(year) ? 29 : 28;
        case 3: return 31;
        case 4: return 30;
        case 5: return 31;
        case 6: return 30;
        case 7: return 31;
        case 8: return 31;
        case 9: return 30;
        case 10: return 31;
        case 11: return 30;
        case 12: return 31;
        default: return 0;
    }
}

// 获取某年某月的第一天是星期几
int getFirstDayOfMonth(int year, int month) {
    tm time_in = {};
    time_in.tm_year = year - 1900;  // 年份从 1900 开始
    time_in.tm_mon = month - 1;     // 月份从 0 开始
    time_in.tm_mday = 1;            // 第一天

    time_t time_temp = mktime(&time_in);
    tm* time_out = localtime(&time_temp);
    return time_out->tm_wday;       // 返回星期几, 0:星期日, 6:星期六
}

// 打印某月的日历
void printMonthCalendar(int month, int year) {
    const char* monthNames[] = { "January", "February", "March", "April", "May", "June",
                                 "July", "August", "September", "October", "November", "December" };

    cout << "   " << monthNames[month - 1] << " " << year << endl;
    cout << " S  M  T  W  T  F  S\n";

    int firstDay = getFirstDayOfMonth(year, month);
    int daysInMonth = getDaysInMonth(month, year);

    // 打印日历
    for (int i = 0; i < firstDay; ++i) {
        cout << "   "; // 根据第一天是星期几，前面填充空白
    }
    for (int day = 1; day <= daysInMonth; ++day) {
        cout << setw(2) << day << " "; // 打印日期
        if ((day + firstDay) % 7 == 0) { // 换行
            cout << endl;
        }
    }
    cout << endl;
}

int main() {
    int year, month;

    cout << "Enter year: ";
    cin >> year;
    cout << "Enter month (1-12): ";
    cin >> month;

    if (month < 1 || month > 12) {
        cerr << "Invalid month!" << endl;
        return 1;
    }

    printMonthCalendar(month, year);
    return 0;
}
