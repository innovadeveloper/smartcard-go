#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdexcept>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>
#include <climits>

using Byte = unsigned char;

using Word = unsigned short;

using UInt = unsigned int;

using LongUInt = uint64_t;

using FullDateInt = unsigned int;

using DateInt = unsigned short;

using ShortTimeInt = unsigned short;

using ByteVector = std::vector<Byte>;

using Mask = ByteVector;

using DateTime_EN_1545 = ByteVector;

using Digits = std::string;

constexpr size_t DATE_EN1545_BUFFER_SIZE {2U};
constexpr size_t TIME_EN1545_BUFFER_SIZE {2U};
constexpr size_t TIME_PTOOLS_BUFFER_SIZE {3U};
constexpr size_t DATETIME_EN1545_BUFFER_SIZE {DATE_EN1545_BUFFER_SIZE + TIME_EN1545_BUFFER_SIZE};

constexpr Byte MAX_SECONDS {59U};
constexpr Byte MAX_MINUTES {59U};
constexpr Byte MAX_HOURS {23U};
constexpr Byte MAX_DAY {31U};
constexpr Byte MAX_MONTH {12U};

// Changing begining to 2000
static const Word BASE_PTOOLS_YEAR {2000U};
static const Word BASE_EN1545_YEAR {1990U};

inline Byte DaysInMonth(Byte month, Word year)
{
  // If it is an Even month, before July
  if (((month % 2 == 0) && (month < 7))) {
    // Febrary
    if (month == 2) {
      // If it's a Leap year
      if (year % 4 == 0 && year % 100 != 0) {
        return 29U;
      }
      return 28U;
    }
    return 30U;
  }

  // If it is an Odd month, after July
  if (((month % 2 != 0) && (month > 7))) {
    return 30U;
  }
  return 31U;
}

/**
 * Shift of bits to left in a vector of bytes. Little endian is requested
 */
static ByteVector LeftShift(const ByteVector& in, size_t shift, bool ZeroOffset = true)
{
  const size_t shiftInsideByte {shift % CHAR_BIT};
  const size_t carrySize {CHAR_BIT - shiftInsideByte};
  const size_t bytesShift {shift / CHAR_BIT};

  Byte carry {0U};
  ByteVector out {ByteVector(bytesShift, ZeroOffset ? 0U : 0xFFU)};

  for (auto b : in) {
    out.push_back(static_cast<Byte>((b << shiftInsideByte) + carry));
    carry = (b >> carrySize);
  }
  // Add last byte
  out.push_back(carry);

  return out;
}

/**
 * Kind of file that can be defined in DESFire EV2/EV3 cards
 */
namespace nxp
{
  enum class FileType { STANDARD, BACKUP, CYCLIC_RECORD, VALUE };

  // File type, File number, File/record size, nb of records (0 if not record file)
  using FileInfo = std::tuple<FileType, Byte, UInt, UInt>;
  const size_t TYPE {0U};
  const size_t INDEX {1U};
  const size_t SIZE {2U};
  const size_t NB_RECS {3U};
} // namespace nxp

/**
 * This type is a Base Type for dates
 */
struct Date
{
  Byte day;
  Byte month;
  Word year;

  /**
   * Constructor
   */
  Date(void) : day {0U}, month {0U}, year {0U}
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `y` -> year
   *         `m` -> moth
   *         `d` -> day
   */
  Date(const Word y, const Byte m, const Byte d) : day {d}, month {m}, year {y}
  {
    // do nothing
  }

  /**
   * Operators
   */
  bool operator==(const Date& date1) const
  {
    return ((day == date1.day) && (month == date1.month) && (year == date1.year));
  };

  bool operator!=(const Date& date1) const { return (!(*this == date1)); }

  bool operator<(const Date& date1) const
  {
    // compare year
    if (year < date1.year) {
      return true;
    }
    else if (year > date1.year) {
      return false;
    }

    // if equal, compare month
    if (month < date1.month) {
      return true;
    }
    else if (month > date1.month) {
      return false;
    }

    // if equal, compare days
    if (day < date1.day) {
      return true;
    }
    return false;
  }

  bool operator<=(const Date& date1) const { return ((*this < date1) || (*this == date1)); }

  bool operator>(const Date& date1) const { return (!(*this <= date1)); }

  bool operator>=(const Date& date1) const { return ((*this > date1) || (*this == date1)); }

  Date operator+(const Date& date1) const
  {
    Date result {year, month, day};

    // Adition of years
    result.year += date1.year;
    result.day = day;
    // if in Febrary of a non Leap Year, me change to 1 of March
    if (result.day > DaysInMonth(month, result.year)) {
      result.day -= DaysInMonth(month, result.year);
      result.month++;
    }

    // Adition of months
    result.month += date1.month;
    // if we have more than 12 months we add a year
    while (result.month > MAX_MONTH) {
      result.year++;
      result.month -= MAX_MONTH;
    }

    result.day += date1.day;
    // if days is biger than the current maximun
    while (result.day > DaysInMonth(result.month, result.year)) {
      result.day -= DaysInMonth(result.month, result.year);
      result.month++;

      while (result.month > MAX_MONTH) {
        result.year++;
        result.month -= MAX_MONTH;
      }
    }

    return result;
  }

  Date operator-(const Date& date1) const
  {
    Date result {year, month, day};

    // Substraction of years
    if (year < date1.year) {
      throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
    }
    result.year -= date1.year;

    // Substraction of months
    //  if months is biger than the current maximun
    if (month < date1.month) {
      if (result.year == 0U) {
        throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
      }
      result.month += MAX_MONTH;
      result.year--;
    }
    result.month -= date1.month;

    // Substraction of days
    //  if days is biger than the current maximun
    while (result.day < date1.day) {

      if (result.month == 0U) {
        result.day += DaysInMonth(MAX_MONTH, result.year);
        if (result.year == 0U) {
          throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
        }
        result.month = MAX_MONTH;
        result.year--;
      }
      else {
        result.day += DaysInMonth(result.month, result.year);
      }
      result.month--;
    }
    result.day -= date1.day;

    return result;
  }
};

/**
 * This type of data is `FECHA` in TICMOVEO/PALMATOOLS documents
 * (5/4/6) dddddmmmmyyyyyy 15 bits
 */
struct DatePTools : virtual public Date
{
  static const size_t DAY_BITSIZE {5U};
  static const size_t MONTH_BITSIZE {4U};
  static const size_t YEAR_BITSIZE {6U};
  static const size_t DATE_BITSIZE {DAY_BITSIZE + MONTH_BITSIZE + YEAR_BITSIZE};

private:
  static const Word YEAR_MASK {0x003FU};
  static const Word MONTH_MASK {0x03C0U};
  static const Word DAY_MASK {0x7C00U};

  static const Word MAX_YEAR {2063U}; // 6bits to 1 = 0x3F = 63

public:
  /**
   * Constructor
   */
  DatePTools(void) : Date {}
  {
    // do nothing
  }

  /**
   * Base Constructor
   */
  explicit DatePTools(const Word y, const Byte m, const Byte d) : Date {y, m, d}
  {
    // do nothing
  }

  /**
   * Constructor copy
   */
  explicit DatePTools(const Date& date) : Date {date.year, date.month, date.day}
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `buffer` -> 2byte buffer with encoded date
   */
  explicit DatePTools(const DateTime_EN_1545& buffer)
  {
    if (buffer.size() != DATE_EN1545_BUFFER_SIZE) {
      throw std::domain_error("DatePTools buffer is not correct");
    }

    Word numberDate = static_cast<Word>((buffer[1] << 8) | buffer[0]);
    day = static_cast<Byte>((numberDate & DAY_MASK) >> (YEAR_BITSIZE + MONTH_BITSIZE));
    if (day > MAX_DAY)
      throw std::out_of_range("Day out of range. Cannot be higher than 31");

    month = static_cast<Byte>((numberDate & MONTH_MASK) >> YEAR_BITSIZE);
    if (month > MAX_MONTH)
      throw std::out_of_range("Month out of range. Cannot be higher than 12");

    year = static_cast<Word>((numberDate & YEAR_MASK) + BASE_PTOOLS_YEAR);
    if (year > MAX_YEAR)
      throw std::out_of_range("Year out of range. Cannot be higher than 2063");
  }

  /**
   * Constructor
   * params: `numberDate` -> 2byte number with encoded date
   */
  explicit DatePTools(const Word numberDate)
  {
    day = static_cast<Byte>((numberDate & DAY_MASK) >> (YEAR_BITSIZE + MONTH_BITSIZE));
    if (day > MAX_DAY)
      throw std::out_of_range("Day out of range. Cannot be higher than 31");

    month = static_cast<Byte>((numberDate & MONTH_MASK) >> YEAR_BITSIZE);
    if (month > MAX_MONTH)
      throw std::out_of_range("Month out of range. Cannot be higher than 12");

    year = static_cast<Word>((numberDate & YEAR_MASK) + BASE_PTOOLS_YEAR);
    if (year > MAX_YEAR)
      throw std::out_of_range("Year out of range. Cannot be higher than 2063");
  }

  /**
   * Returns a byte vector with date encoded
   */
  const ByteVector GetCurrentBufferDate(void) const
  {
    const Byte byte0 {static_cast<Byte>((year - BASE_PTOOLS_YEAR) + static_cast<Byte>(month << 6U))};
    const Byte byte1 {static_cast<Byte>((month >> 2U) + (day << 2U))};
    return ByteVector {byte0, byte1};
  }

  /**
   * Returns a 2byte vector with date encoded
   */
  const Word GetCurrentNumberDate(void) const
  {
    if (day > MAX_DAY) {
      throw(std::out_of_range("Day out of range. Cannot be higher than 31"));
    }
    if (month > MAX_MONTH) {
      throw(std::out_of_range("Month out of range. Cannot be higher than 12"));
    }
    if (year > MAX_YEAR) {
      throw(std::out_of_range("Year cannot be higher than 63"));
    }
    return (year - BASE_PTOOLS_YEAR) | (month << YEAR_BITSIZE) | day << ((YEAR_BITSIZE + MONTH_BITSIZE));
  };
};

/**
 * This type of data is Date Compact based on EN1545.
 * (7/4/5) yyyyyyymmmmddddd
 */
struct DateEN1545 : virtual public Date
{
  static const size_t DAY_BITSIZE {5U};
  static const size_t MONTH_BITSIZE {4U};
  static const size_t YEAR_BITSIZE {7U};
  static const size_t DATE_BITSIZE {DAY_BITSIZE + MONTH_BITSIZE + YEAR_BITSIZE};

private:
  static const Word YEAR_MASK {0xFE00U};
  static const Word MONTH_MASK {0x01E0U};
  static const Word DAY_MASK {0x001FU};

  static const Word MAX_YEAR {2127U}; // 1bits to 1 = 0x7F = 127

public:
  DateEN1545(void) : Date {}
  {
    // do nothing
  }

  /**
   * Base Constructor
   */
  explicit DateEN1545(const Word y, const Byte m, const Byte d) : Date {y, m, d}
  {
    // do nothing
  }

  /**
   * Constructor copy
   */
  explicit DateEN1545(const Date& date) : Date {date.year, date.month, date.day}
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `buffer` -> 2byte buffer with encoded date
   */
  explicit DateEN1545(const DateTime_EN_1545& buffer)
  {
    if (buffer.size() != DATE_EN1545_BUFFER_SIZE) {
      throw std::domain_error("DateEN1545 buffer is not correct");
    }

    Word numberDate = static_cast<Word>((buffer[1] << 8) | buffer[0]);
    day = static_cast<Byte>(numberDate & DAY_MASK);
    if (day > MAX_DAY)
      throw std::out_of_range("Day out of range. Cannot be higher than 31");

    month = static_cast<Byte>((numberDate & MONTH_MASK) >> DAY_BITSIZE);
    if (month > MAX_MONTH)
      throw std::out_of_range("Month out of range. Cannot be higher than 12");

    year = static_cast<Word>(((numberDate & YEAR_MASK) >> (MONTH_BITSIZE + DAY_BITSIZE)) + BASE_EN1545_YEAR);
    if (year > MAX_YEAR)
      throw std::out_of_range("Year out of range. Cannot be higher than 2117");
  }

  /**
   * Constructor
   * params: `numberDate` -> 2byte number with encoded date
   */
  explicit DateEN1545(const Word numberDate)
  {
    day = static_cast<Byte>(numberDate & DAY_MASK);
    if (day > MAX_DAY)
      throw std::out_of_range("Day out of range. Cannot be higher than 31");

    month = static_cast<Byte>((numberDate & MONTH_MASK) >> DAY_BITSIZE);
    if (month > MAX_MONTH)
      throw std::out_of_range("Month out of range. Cannot be higher than 12");

    year = static_cast<Word>(((numberDate & YEAR_MASK) >> (MONTH_BITSIZE + DAY_BITSIZE)) + BASE_EN1545_YEAR);
    if (year > MAX_YEAR)
      throw std::out_of_range("Year out of range. Cannot be higher than 2117");
  }

  /**
   * Returns a byte vector with date encoded
   */
  const ByteVector GetCurrentBufferDate(void) const
  {
    const Byte byte0 {static_cast<Byte>(day + static_cast<Byte>(month << 5U))};
    const Byte byte1 {static_cast<Byte>((month >> 3U) + ((year - BASE_EN1545_YEAR) << 1U))};
    return ByteVector {byte0, byte1};
  }

  /**
   * Returns a 2byte vector with date encoded
   */
  const Word GetCurrentNumberDate(void) const
  {
    if (day > MAX_DAY) {
      throw(std::out_of_range("Day out of range. Cannot be higher than 31"));
    }
    if (month > MAX_MONTH) {
      throw(std::out_of_range("Month out of range. Cannot be higher than 12"));
    }
    if ((year - BASE_EN1545_YEAR) > MAX_YEAR) {
      throw(std::out_of_range("Year cannot be higher than 2127"));
    }
    return day | (month << DAY_BITSIZE) | (year - BASE_EN1545_YEAR) << (DAY_BITSIZE + MONTH_BITSIZE);
  };
};

/**
 * This type of data is `TIME`
 */
struct Time
{
public:
  Byte sec;
  Byte min;
  Byte hour;

public:
  /**
   * Constructor
   */
  Time(void) :
      sec {0U}, min {0U}, hour {0U} {
        // do nothing
      };

  /**
   * Constructor
   * params: `h` -> hours
   *         `m` -> minutes
   *         `s` -> seconds
   */
  Time(const Byte h, const Byte m, const Byte s) : sec(s), min(m), hour(h)
  {
    // do nothing
  }

  /**
   * Operators
   */
  bool operator==(const Time& time1) const { return (sec == time1.sec) && (min == time1.min) && (hour == time1.hour); };

  bool operator!=(const Time& time1) const { return (!(*this == time1)); }

  bool operator<(const Time& time1) const
  {
    // compare hour
    if (hour < time1.hour) {
      return true;
    }
    else if (hour > time1.hour) {
      return false;
    }

    // if equal, compare month
    if (min < time1.min) {
      return true;
    }
    else if (min > time1.min) {
      return false;
    }

    // if equal, compare days
    if (sec < time1.sec) {
      return true;
    }
    return false;
  }

  bool operator<=(const Time& time1) const { return ((*this < time1) || (*this == time1)); }

  bool operator>(const Time& time1) const { return (!(*this <= time1)); }

  bool operator>=(const Time& time1) const { return ((*this > time1) || (*this == time1)); }

  Time operator+(const Time& time1) const
  {
    Time result {hour, min, sec};
    result.sec += time1.sec;
    // seconds to min
    if (result.sec > MAX_SECONDS) {
      result.min++;
      result.sec -= (MAX_SECONDS + 1U);
    }

    result.min += time1.min;
    // mins to hours
    while (result.min > MAX_MINUTES) {
      result.hour++;
      result.min -= (MAX_MINUTES + 1U);
    }

    result.hour += time1.hour;

    return result;
  }

  Time operator-(const Time& time1) const
  {
    Time result;

    // not enough hours to deduct
    if (hour < time1.hour) {
      throw std::out_of_range("Bad difference operation. Hour cannot be less than 0");
    }
    result.hour = hour - time1.hour;

    // if not enougth min to deduct -1 hour
    if (min < time1.min) {
      result.min = min + (MAX_MINUTES + 1U) - time1.min;
      if (result.hour == 0) {
        throw std::out_of_range("Bad difference operation. Hour cannot be less than 0");
      }
      result.hour--;
    }
    else {
      result.min = min - time1.min;
    }

    // if not enougth sec to deduct -1 min
    if (sec < time1.sec) {
      result.sec = sec + (MAX_SECONDS + 1U) - time1.sec;
      if (result.min == 0) {
        result.min += (MAX_MINUTES + 1U);
        if (result.hour == 0) {
          throw std::out_of_range("Bad difference operation. Hour cannot be less than 0");
        }
        result.hour--;
      }
      result.min--;
    }
    else {
      result.sec = sec - time1.sec;
    }

    return result;
  }
};

/**
 * This type of data is Time Compact based on PToolsFormat.
 * (6/6/5) ssssssmmmmmmhhhhh
 */
struct TimePTools : virtual public Time
{
  static const size_t SECONDS_BITSIZE {6U};
  static const size_t MINUTES_BITSIZE {6U};
  static const size_t HOURS_BITSIZE {5U};
  static const size_t TIME_BITSIZE {HOURS_BITSIZE + MINUTES_BITSIZE + SECONDS_BITSIZE};

protected:
  static const UInt HOURS_MASK {0x1F}; // Last 5 bits
  static const UInt MINUTES_MASK {0x7E0U}; // Next 6 bits after hours
  static const UInt SECONDS_MASK {0x1F800U}; // Next 6 bits after minutes

public:
  /**
   * Constructor
   */
  TimePTools(void) :
      Time {} {
        // do nothing
      };

  /**
   * Base Constructor
   */
  TimePTools(const Byte h, const Byte m, const Byte s) :
      Time {h, m, s} {
        // do nothing
      };

  /**
   * Constructor copy
   * params: `time`
   */
  explicit TimePTools(const Time& time) : Time {time.hour, time.min, time.sec}
  {
    // do nothing
  }

  /**
   * Constructor with bufer
   * params: `buffer`
   */
  explicit TimePTools(const DateTime_EN_1545& buffer)
  {
    if (buffer.size() != TIME_PTOOLS_BUFFER_SIZE) {
      throw std::domain_error("TimePTools buffer is not correct");
    }

    UInt numberTime = static_cast<UInt>((buffer[2] << 16) | (buffer[1] << 8) | buffer[0]);
    // Real seconds have to be
    hour = static_cast<Byte>(numberTime & HOURS_MASK);
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }

    min = static_cast<Byte>((numberTime & MINUTES_MASK) >> HOURS_BITSIZE);
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }

    sec = static_cast<Byte>((numberTime & SECONDS_MASK) >> (HOURS_BITSIZE + MINUTES_BITSIZE));
    if (sec > MAX_SECONDS) {
      throw(std::out_of_range("Seconds cannot be higher than 59"));
    }
  }

  /**
   * Constructor
   * params: `numberTime` -> encoded `Time` 4byte number
   */
  explicit TimePTools(UInt numberTime)
  {
    hour = static_cast<Byte>(numberTime & HOURS_MASK);
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }

    min = static_cast<Byte>((numberTime & MINUTES_MASK) >> HOURS_BITSIZE);
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }

    sec = static_cast<Byte>((numberTime & SECONDS_MASK) >> (HOURS_BITSIZE + MINUTES_BITSIZE));
    if (sec > MAX_SECONDS) {
      throw(std::out_of_range("Seconds cannot be higher than 59"));
    }
  }

  /**
   * Returns a 4byte number that represents time
   */
  UInt GetCurrentNumberTime(void) const
  {
    if (sec > MAX_SECONDS) {
      throw(std::out_of_range("Seconds cannot be higher than 59"));
    }
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }
    return static_cast<UInt>(hour | (min << HOURS_BITSIZE) | (sec << (HOURS_BITSIZE + MINUTES_BITSIZE)));
  }

  /**
   * Returns a byte vector with time encoded
   */
  const ByteVector GetCurrentBufferTime(void) const
  {
    const Byte byte0 {static_cast<Byte>(static_cast<Byte>(hour) + static_cast<Byte>(min << 5U))};
    const Byte byte1 {static_cast<Byte>((min >> 3U) + (sec << 3U))};
    const Byte byte2 {static_cast<Byte>(sec >> 5U)};
    return ByteVector {byte0, byte1, byte2};
  }

  /**
   * As seconds are 2 fraction, it's divided by 2 so equality would accomplish for even and
   * next odd
   */
  bool operator==(const TimePTools& time1) const
  {
    return (sec == time1.sec) && (min == time1.min) && (hour == time1.hour);
  };
};

/**
 * This type of data is a Compact version of Time where seconds equal to 0.
 * (6/5) mmmmmmmhhhhh
 */
struct ShortTime : public TimePTools
{
public:
  static const size_t MINUTES_BITSIZE {6U};
  static const size_t HOURS_BITSIZE {5U};
  static const size_t SHORT_TIME_BITSIZE {HOURS_BITSIZE + MINUTES_BITSIZE};

protected:
  static const UInt HOURS_MASK {0x1F}; // Last 5 bits
  static const UInt MINUTES_MASK {0x7E0U}; // Next 6 bits after hours

public:
  /**
   * Constructor
   */
  ShortTime(void) : Time(), TimePTools()
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `h` -> hour
   *         `m` -> minute
   */
  ShortTime(const Byte h, const Byte m) : Time(h, m, 0U), TimePTools(h, m, 0U)
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `numberTime` -> encoded `Time` 4byte number
   */
  explicit ShortTime(UInt numberTime) : Time(), TimePTools()
  {
    hour = static_cast<Byte>(numberTime & HOURS_MASK);
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }

    min = static_cast<Byte>((numberTime & MINUTES_MASK) >> HOURS_BITSIZE);
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }

    sec = 0U;
  }

  /**
   * Returns a 2byte number that represents time
   */
  Word GetCurrentNumberShortTime(void) const
  {
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }
    return static_cast<Word>(hour | (min << HOURS_BITSIZE));
  }
};


/**
 * This type of data is Time Compact based on EN1545.
 * (5/6/5) hhhhhMMMMMMsssss
 * Seconds are counted each 2, so real seconds are *2
 */
struct TimeEN1545 : virtual public Time
{
  static const size_t SECONDS_BITSIZE {5U};
  static const size_t MINUTES_BITSIZE {6U};
  static const size_t HOURS_BITSIZE {5U};
  static const size_t TIME_BITSIZE {HOURS_BITSIZE + MINUTES_BITSIZE + SECONDS_BITSIZE};

protected:
  static const UInt SECONDS_MASK {0x1F}; // Last 5 bits
  static const UInt MINUTES_MASK {0x7E0U}; // Next 6 bits after hours
  static const UInt HOURS_MASK {0x1F800U}; // Next 6 bits after minutes

public:
  /**
   * Constructor
   */
  TimeEN1545(void) :
      Time {} {
        // do nothing
      };

  /**
   * Base Constructor
   */
  TimeEN1545(const Byte h, const Byte m, const Byte s) :
      Time {h, m, s} {
        // do nothing
      };

  /**
   * Constructor copy
   * params: `time`
   */
  explicit TimeEN1545(const Time& time) : Time {time.hour, time.min, time.sec}
  {
    // do nothing
  }

  /**
   * Constructor with bufer
   * params: `buffer`
   */
  explicit TimeEN1545(const DateTime_EN_1545& buffer)
  {
    if (buffer.size() != TIME_EN1545_BUFFER_SIZE) {
      throw std::domain_error("TimeEN1545 buffer is not correct");
    }

    Word numberTime = static_cast<Word>((buffer[1] << 8) | buffer[0]);
    // Real seconds have to be
    sec = (2 * static_cast<Byte>(numberTime & SECONDS_MASK));
    if (sec > MAX_SECONDS) {
      throw(std::out_of_range("Seconds cannot be higher than 59"));
    }
    min = static_cast<Byte>((numberTime & MINUTES_MASK) >> SECONDS_BITSIZE);
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }
    hour = static_cast<Byte>((numberTime & HOURS_MASK) >> (SECONDS_BITSIZE + MINUTES_BITSIZE));
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }
  }

  /**
   * Constructor
   * params: `numberTime` -> encoded `Time` 4byte number
   */
  explicit TimeEN1545(UInt numberTime)
  {
    sec = (2 * static_cast<Byte>(numberTime & SECONDS_MASK));
    if (sec > MAX_SECONDS) {
      throw(std::out_of_range("Seconds cannot be higher than 59"));
    }
    min = static_cast<Byte>((numberTime & MINUTES_MASK) >> SECONDS_BITSIZE);
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }
    hour = static_cast<Byte>((numberTime & HOURS_MASK) >> (SECONDS_BITSIZE + MINUTES_BITSIZE));
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }
  }

  /**
   * Returns a 4byte number that represents time
   */
  UInt GetCurrentNumberTime(void) const
  {
    if (sec > MAX_SECONDS) {
      throw(std::out_of_range("Seconds cannot be higher than 59"));
    }
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }
    return static_cast<UInt>(hour | (min << HOURS_BITSIZE) | (sec << (HOURS_BITSIZE + MINUTES_BITSIZE)));
  }

  /**
   * Returns a byte vector with time encoded
   */
  const ByteVector GetCurrentBufferTime(void) const
  {
    const Byte byte0 {static_cast<Byte>(static_cast<Byte>(sec / 2U) + static_cast<Byte>(min << 5U))};
    const Byte byte1 {static_cast<Byte>((min >> 3U) + (hour << 3U))};
    return ByteVector {byte0, byte1};
  }

  /**
   * As seconds are 2 fraction, it's divided by 2 so equality would accomplish for even and
   * next odd
   */
  bool operator==(const TimeEN1545& time1) const
  {
    return (sec / 2U == time1.sec / 2U) && (min == time1.min) && (hour == time1.hour);
  };
};

/**
 * This type of data is Time Compact based on EN1545, but without seconds.
 * (5/6) hhhhhMMMMMM
 */
struct ShortTimeEN1545 : public TimeEN1545
{
public:
  static const size_t MINUTES_BITSIZE {6U};
  static const size_t HOURS_BITSIZE {5U};
  static const size_t SHORT_TIME_BITSIZE {HOURS_BITSIZE + MINUTES_BITSIZE};

protected:
  static const UInt MINUTES_MASK {0x3F}; // Last 6 bits
  static const UInt HOURS_MASK {0x7C0U}; // Next 5 bits after hours 0x7C0U

public:
  /**
   * Constructor
   */
  ShortTimeEN1545(void) : Time(), TimeEN1545()
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `h` -> hour
   *         `m` -> minute
   */
  ShortTimeEN1545(const Byte h, const Byte m) : Time(h, m, 0U), TimeEN1545(h, m, 0U)
  {
    // do nothing
  }

  /**
   * Constructor
   * params: `numberTime` -> encoded `Time` 4byte number
   */
  explicit ShortTimeEN1545(UInt numberTime) : Time(), TimeEN1545()
  {
    hour = static_cast<Byte>((numberTime & HOURS_MASK) >> MINUTES_BITSIZE);
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }

    min = static_cast<Byte>(numberTime & MINUTES_MASK);
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }

    sec = 0U;
  }

  /**
   * Returns a 2byte number that represents time
   */
  Word GetCurrentNumberShortTime(void) const
  {
    if (min > MAX_MINUTES) {
      throw(std::out_of_range("Minutes cannot be higher than 59"));
    }
    if (hour > MAX_HOURS) {
      throw(std::out_of_range("Hour cannot be higher than 23"));
    }
    return static_cast<Word>(min | (hour << MINUTES_BITSIZE));
  }
};

/**
 * This type of data
 */
struct FullDate : virtual public Date, virtual public Time
{
public:
  /**
   * Constructor
   */
  FullDate(void) :
      Date {}, Time {} {
        // do nothing
      };

  /**
   * Constructor
   * params: `date`
   *         `time`
   */
  FullDate(Date date, Time time) : Date {date}, Time {time} {};

  /**
   * Operators
   */
  bool operator==(const FullDate& datetime1) const
  {
    return (Date::operator==(datetime1) && (Time::operator==(datetime1)));
  };

  bool operator!=(const FullDate& datetime1) const { return (!(*this == datetime1)); };

  bool operator<(const FullDate& datetime1) const
  {
    // compare date
    if (Date::operator<(datetime1)) {
      return true;
    }
    else if (Date::operator>(datetime1)) {
      return false;
    }

    // if equal, compare time
    if (Time::operator<(datetime1)) {
      return true;
    }
    else if (Time::operator>(datetime1)) {
      return false;
    }

    // if equal, false
    return false;
  }

  bool operator<=(const FullDate& datetime1) const { return ((*this < datetime1) || (*this == datetime1)); }

  bool operator>(const FullDate& datetime1) const { return (!(*this <= datetime1)); }

  bool operator>=(const FullDate& datetime1) const { return ((*this > datetime1) || (*this == datetime1)); }

  FullDate operator+(const FullDate& datetime1) const
  {
    FullDate result {Date {year, month, day}, Time {hour, min, sec}};

    // Seconds addition
    result.sec += datetime1.sec;
    // secs to min
    if (result.sec > MAX_SECONDS) {
      result.min++;
      result.sec -= (MAX_SECONDS + 1U);
    }

    // Minutes addition
    result.min += datetime1.min;
    while (result.min > MAX_MINUTES) {
      // mins to hour
      result.hour++;
      result.min -= (MAX_MINUTES + 1U);
    }

    // Hours addition
    result.hour += datetime1.hour;
    // hours to day
    while (result.hour > MAX_HOURS) {
      result.day++;
      result.hour -= (MAX_HOURS + 1U);
    }

    // Days addition
    result.day += datetime1.day;
    // if days is biger than the current maximun
    while (result.day > DaysInMonth(result.month, result.year)) {
      result.day -= DaysInMonth(result.month, result.year);
      result.month++;
      while (result.month > MAX_MONTH) {
        result.year++;
        result.month -= MAX_MONTH;
      }
    }

    // Months addition
    result.month += datetime1.month;
    // if we have more than 12 months we add a year
    while (result.month > MAX_MONTH) {
      result.year++;
      result.month -= MAX_MONTH;
    }

    // Years addition
    result.year += datetime1.year;
    // if in Febrary of a non Leap Year, we change to 1 of March
    if (result.day > DaysInMonth(month, result.year)) {
      result.day -= DaysInMonth(month, result.year);
      result.month++;
      if (result.month > MAX_MONTH) {
        result.year++;
        result.month -= MAX_MONTH;
      }
    }

    return result;
  }

  FullDate operator-(const FullDate& datetime1) const
  {
    FullDate result {Date {year, month, day}, Time {hour, min, sec}};

    // Substraction of years
    if (year < datetime1.year) {
      throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
    }
    result.year -= datetime1.year;

    // Substraction of months

    if (month < datetime1.month) {
      if (result.year == 0U) {
        throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
      }
      result.month += MAX_MONTH;
      result.year--;
    }
    result.month -= datetime1.month;

    // Substraction of days
    // if days is biger than the current maximun
    while (result.day < datetime1.day) {

      if (result.month == 0U) {
        result.day += DaysInMonth(MAX_MONTH, result.year);
        if (result.year == 0U) {
          throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
        }
        result.month = MAX_MONTH;
        result.year--;
      }
      else {
        result.day += DaysInMonth(result.month, result.year);
      }
      result.month--;
    }
    result.day -= datetime1.day;

    // Substraction of hours
    // if hours is biger than the current maximun
    if (result.hour < datetime1.hour) {
      result.hour += (MAX_HOURS + 1);
      if (result.day == 0U) {
        if (result.month == 0U) {
          result.day += DaysInMonth(MAX_MONTH, result.year) - 1U;
          if (result.year == 0U) {
            throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
          }
          result.month = MAX_MONTH;
          result.year--;
        }
        else {
          result.day = DaysInMonth((result.month - 1U), result.year);
        }
        result.month--;
      }
      else {
        result.day--;
      }
    }
    result.hour -= datetime1.hour;

    // Substraction of minutes
    // if min is biger than the current maximun
    if (min < datetime1.min) {
      result.min += (MAX_MINUTES + 1U);
      if (result.hour == 0) {
        result.hour = MAX_HOURS;
        if (result.day == 0U) {
          if (result.month == 0U) {
            if (result.year == 0U) {
              throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
            }
            result.day += DaysInMonth(MAX_MONTH, result.year);
            result.month = MAX_MONTH;
            result.year--;
          }
          else {
            result.day += DaysInMonth((result.month - 1U), result.year);
          }
          result.month--;
        }
        result.day--;
      }
      else {
        result.hour--;
      }
    }
    result.min -= datetime1.min;

    // Substraction of seconds
    // if seconds is biger than the current maximun
    if (result.sec < datetime1.sec) {
      result.sec += (MAX_SECONDS + 1U);
      if (result.min == 0) {
        result.min += (MAX_MINUTES + 1U);
        if (result.hour == 0) {
          result.hour = MAX_HOURS;
          if (result.day == 0U) {
            if (result.month == 0U) {

              if (result.year == 0U) {
                throw std::out_of_range("Bad difference operation. Year cannot be less than 2000");
              }
              result.day += DaysInMonth(MAX_MONTH, result.year);
              result.month = MAX_MONTH;
              result.year--;
            }
            else {
              result.day += DaysInMonth((result.month - 1U), result.year);
            }

            result.month--;
          }
          result.day--;
        }
        else {
          result.hour--;
        }
      }
      result.min--;
    }
    result.sec -= datetime1.sec;

    return result;
  }
};

/**
 * This type of data is `FECHA COMPLETA` in TICMOVEO/PALMATOOLS documents
 * (6/6/5/5/4/6) ssssssmmmmmmhhhhh dddddmmmmyyyyyy
 */
struct DateTimePTools : public FullDate, public DatePTools, public TimePTools
{
public:
  static const size_t FULLDATE_BITSIZE {DATE_BITSIZE + SECONDS_BITSIZE + MINUTES_BITSIZE + HOURS_BITSIZE};

private:
  static constexpr UInt DATE_BITSIZE = 15U;
  static constexpr UInt TIME_BITSIZE = 17U;
  static constexpr UInt DATE_MASK = (1U << DATE_BITSIZE) - 1;
  static constexpr UInt TIME_MASK = (1U << TIME_BITSIZE) - 1;

public:
  /**
   * Constructor
   */
  DateTimePTools(void) :
      DatePTools {}, TimePTools {} {
        // do nothing
      };

  /**
   * Constructor
   */
  DateTimePTools(const DateTime_EN_1545& buffer) :
      DateTimePTools(static_cast<UInt>((buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0]))
  {
    if (buffer.size() != DATETIME_EN1545_BUFFER_SIZE) {
      throw std::domain_error("DateTimePTools buffer is not correct");
    }
  };

  /**
   * Constructor
   * params: `date`
   *         `time`
   */
  DateTimePTools(Date date, Time time) :
      // Since Date and Time are virtual bases, they must be initialized explicitly here
      Date {date.year, date.month, date.day}, Time {time.hour, time.min, time.sec}, FullDate {date, time},
      DatePTools {date}, TimePTools {time}
  {
  }

  explicit DateTimePTools(const FullDate& fullDate) :
      Date {fullDate.year, fullDate.month, fullDate.day}, Time {fullDate.hour, fullDate.min, fullDate.sec},
      DatePTools {fullDate}, TimePTools {fullDate}
  {
    // do nothing
  }

  explicit DateTimePTools(const UInt numberFullDate) :
      DatePTools {static_cast<Word>(numberFullDate & DATE_MASK)},
      TimePTools {(numberFullDate >> DATE_BITSIZE) & TIME_MASK}
  {
    // do nothing
  }

  /**
   * Returns a 4byte number that represents date and time
   */
  const UInt GetCurrentNumberFullDate(void) const
  {
    UInt timeBits = GetCurrentNumberTime();
    UInt dateBits = GetCurrentNumberDate();
    return (timeBits << DATE_BITSIZE) | dateBits;
  };

  /**
   * Returns a 4byte number that represents date and time
   */
  const ByteVector GetCurrentBufferDateTime(void) const
  {
    
    ByteVector bufferTime {GetCurrentBufferTime()};
    const ByteVector bufferDate {GetCurrentBufferDate()};
    bufferTime = LeftShift(bufferTime, 7);
    const Byte Byte1 {static_cast<Byte>(bufferDate[1] | bufferTime[0])};
    const ByteVector buffer {bufferDate[0],Byte1, bufferTime[1], bufferTime[2]};
    return buffer;
  };

  /**
   * Operators
   */
  bool operator==(const DateTimePTools& datetime1) const
  {
    return (Date::operator==(datetime1) && (Time::operator==(datetime1)));
  };

  bool operator!=(const DateTimePTools& datetime1) const { return (!(*this == datetime1)); };

  bool operator<(const DateTimePTools& datetime1) const
  {
    return FullDate {Date {year, month, day}, Time {hour, min, sec}}
    < FullDate {Date {datetime1.year, datetime1.month, datetime1.day},
                Time {datetime1.hour, datetime1.min, datetime1.sec}};
  }

  bool operator<=(const DateTimePTools& datetime1) const { return ((*this < datetime1) || (*this == datetime1)); }

  bool operator>(const DateTimePTools& datetime1) const { return (!(*this <= datetime1)); }

  bool operator>=(const DateTimePTools& datetime1) const { return ((*this > datetime1) || (*this == datetime1)); }

  DateTimePTools operator+(const DateTimePTools& datetime1) const
  {
    return DateTimePTools {FullDate {Date {year, month, day}, Time {hour, min, sec}}
                           + FullDate {Date {datetime1.year, datetime1.month, datetime1.day},
                                       Time {datetime1.hour, datetime1.min, datetime1.sec}}};
  }

  DateTimePTools operator-(const FullDate& datetime1) const
  {
    return DateTimePTools {FullDate {Date {year, month, day}, Time {hour, min, sec}}
                           - FullDate {Date {datetime1.year, datetime1.month, datetime1.day},
                                       Time {datetime1.hour, datetime1.min, datetime1.sec}}};
  }
};

/**
 * This type of data is DateTimeCompact based on EN1545
 * yyyyyyymmmmdddddhhhhhMMMMMMsssss (7/4/5/5/6/5)
 */
struct DateTimeEN1545 : virtual public DateEN1545, virtual public TimeEN1545, virtual public FullDate
{
  static const size_t FULLDATE_BITSIZE {DATE_BITSIZE + SECONDS_BITSIZE + MINUTES_BITSIZE + HOURS_BITSIZE};

private:
  static constexpr UInt DATE_BITSIZE = 16U;
  static constexpr UInt TIME_BITSIZE = 16U;
  static constexpr UInt DATE_MASK = (1U << DATE_BITSIZE) - 1;
  static constexpr UInt TIME_MASK = (1U << TIME_BITSIZE) - 1;

public:
  /**
   * Constructor
   */
  DateTimeEN1545(void) :
      DateEN1545 {}, TimeEN1545 {} {
        // do nothing
      };
  /**
   * Constructor
   */
  DateTimeEN1545(const DateTime_EN_1545& buffer) :
      TimeEN1545 {ByteVector {buffer.begin(), buffer.begin() + DATE_EN1545_BUFFER_SIZE}},
      DateEN1545 {ByteVector {buffer.begin() + DATE_EN1545_BUFFER_SIZE, buffer.end()}}
  {
    if (buffer.size() != DATETIME_EN1545_BUFFER_SIZE) {
      throw std::domain_error("DateTimeEN1545 buffer is not correct");
    }
  };

  /**
   * Constructor
   * params: `date`
   *         `time`
   */
  DateTimeEN1545(Date date, Time time) :
      // Since Date and Time are virtual bases, they must be initialized explicitly here
      Date {date.year, date.month, date.day}, Time {time.hour, time.min, time.sec}, FullDate {date, time},
      DateEN1545 {date}, TimeEN1545 {time}
  {
  }

  explicit DateTimeEN1545(const FullDate& fullDate) :
      Date {fullDate.year, fullDate.month, fullDate.day}, Time {fullDate.hour, fullDate.min, fullDate.sec},
      DateEN1545 {fullDate}, TimeEN1545 {fullDate}
  {
    // do nothing
  }

  explicit DateTimeEN1545(const UInt numberFullDate) :
      DateEN1545 {static_cast<Word>(numberFullDate >> TIME_BITSIZE)}, TimeEN1545 {static_cast<Word>(numberFullDate)}
  {
    // do nothing
  }

  /**
   * Returns a 4byte number that represents date and time
   */
  const UInt GetCurrentNumberFullDate(void) const
  {
    UInt currentNumberFullDate {static_cast<UInt>(GetCurrentNumberTime())
                                << (YEAR_BITSIZE + MONTH_BITSIZE + DAY_BITSIZE)};
    currentNumberFullDate |= GetCurrentNumberDate();
    return currentNumberFullDate;
  };

  /**
   * Returns a 4byte number that represents date and time
   */
  const ByteVector GetCurrentBufferDateTime(void) const
  {
    ByteVector buffer {GetCurrentBufferTime()};
    const ByteVector bufferDate {GetCurrentBufferDate()};
    buffer.insert(buffer.end(), bufferDate.begin(), bufferDate.end());
    return buffer;
  };

  /**
   * Operators
   */
  bool operator==(const DateTimeEN1545& datetime1) const
  {
    return (DateEN1545::operator==(datetime1) && (TimeEN1545::operator==(datetime1)));
  };

  bool operator!=(const DateTimeEN1545& datetime1) const { return (!(*this == datetime1)); };

  bool operator<(const DateTimeEN1545& datetime1) const
  {
    return FullDate {Date {year, month, day}, Time {hour, min, sec}}
    < FullDate {Date {datetime1.year, datetime1.month, datetime1.day},
                Time {datetime1.hour, datetime1.min, datetime1.sec}};
  }

  bool operator<=(const DateTimeEN1545& datetime1) const { return ((*this < datetime1) || (*this == datetime1)); }

  bool operator>(const DateTimeEN1545& datetime1) const { return (!(*this <= datetime1)); }

  bool operator>=(const DateTimeEN1545& datetime1) const { return ((*this > datetime1) || (*this == datetime1)); }

  DateTimeEN1545 operator+(const DateTimeEN1545& datetime1) const
  {
    return DateTimeEN1545 {FullDate {Date {year, month, day}, Time {hour, min, sec}}
                           + FullDate {Date {datetime1.year, datetime1.month, datetime1.day},
                                       Time {datetime1.hour, datetime1.min, datetime1.sec}}};
  }

  DateTimeEN1545 operator-(const FullDate& datetime1) const
  {
    return DateTimeEN1545 {FullDate {Date {year, month, day}, Time {hour, min, sec}}
                           - FullDate {Date {datetime1.year, datetime1.month, datetime1.day},
                                       Time {datetime1.hour, datetime1.min, datetime1.sec}}};
  }
};

#endif
