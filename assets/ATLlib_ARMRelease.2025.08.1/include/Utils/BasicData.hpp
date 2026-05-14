#ifndef BASICDATA_HPP
#define BASICDATA_HPP

#include "types.hpp"
#include <string>

/**
 * Data type extracted from PalmaTools/TICMOVEO documents
 */
enum class CardDataType {
  /**
   * Text string
   */
  CARACTERES,

  /**
   * Hexadecimal string
   */
  HEXADECIMAL,

  /**
   * Number of different lengths
   */
  NUMERO,

  /**
   * Boolean
   */
  BOOL,

  /**
   * Binary mask string
   */
  MASK,

  /**
   * Date in format (bits representation) dddddMMMMyyyyyy
   */
  FECHA,

  /**
   * Time in format (bits representation) ssssssmmmmmmhhhhh
   */
  HORA,

  /**
   *Time in format (bits representation) mmmmmmhhhhh seconds equal to 0
   * E.g.: 0x4E6 = 06:39:00
   */
  HORA_CORTA,

  /**
   * Timestamp in format (bits representation) ssssssmmmmmmhhhhh + FECHA
   */
  FECHAHORA,

  /**
   * Sucession of numbers, each hexadecimal value is one of them.
   * E.g.: 0x123490 = 123490
   */
  DIGITOS,

  /**
   * Date with norm EN-1545 (5/4/7). 1/1/1990 - 31/12/2117. Format: yyyyyyMMMMddddd
   * E.g.: 39-8A  = 10/12/2018
   */
  FECHA_EN1545,

  /**
   * Time with norm EN-1545 (5/6/5). Format: hhhhhmmmmmmsssss
   * E.g.: 75-BC  = 14:45:55
   */
  HORA_EN1545,

  /**
   *Time in format (bits representation) hhhhhmmmmmm seconds equal to 0
   * E.g.: 0x1A7 = 06:39:00
   */
  HORA_CORTA_EN1545,

  /**
   * Date and time with norm EN-1545 (Date and Time together)
   * E.g.: 39-8A-75-BC = 10/12/2018 14:45:55
   */
  FECHAHORA_EN1545
};

template <typename T>
class BasicData
{
  T Value_;

  /**
   * Array of bytes dad defines data
   */
  ByteVector Buffer_;

  /**
   * Data type according TICMOVEO/PALMA TOOLS documentation
   */
  CardDataType Type_;

  /**
   * Bit position where the current data starts (first is 0)
   * Constructior will convert to 0-starting index.
   */
  const size_t Offset_;

  /**
   * Number of bits involved in data
   */
  const size_t BitLength_;

  /**
   * The mask needed for the first involved byte to ignore unneeded bits
   */
  const Byte MinMask_;

  /**
   * The mask needed for the last involved byte to ignore unneeded bits
   */
  const Byte MaxMask_;

  /**
   * Number of bytes involved in data
   */
  const size_t ByteLength_;

  /**
   * `true`if the current data is defined. `false` otherwise
   */
  bool Defined_;

public:
  /**
   * Constructor
   * params:  Type
   *          Offset -> must be 1 or bigger, according TICMOVE/PALMA TOOLS documentation
   *          BitLenth
   */
  BasicData(const CardDataType Type, const size_t Offset, const size_t BitLength);

  /**
   * Destructor
   */
  ~BasicData(void) = default;

  /**
   * Parses data from a file buffer
   * params: FileData (in)
   */
  void SetDataFromFile(const ByteVector& FileData);

  /**
   * Setter
   */
  void SetValue(const T& Value);

  /**
   * Getter
   */
  const T& GetValue(void) const { return Value_; };

  /**
   * Getter
   */
  std::string GetValueAsString() const;

  /**
   * Getter
   */
  const ByteVector& GetBuffer(void) const { return Buffer_; };

  /**
   * Concatenates current buffer to an external buffer to compose again a file
   */
  void ConcatenateToExternalBuffer(ByteVector& ExtBuffer);

  bool IsDefined(void) const { return Defined_; };

  DateEN1545 GetDateEN1545Value(void) const;

  DatePTools GetDateValue(void) const;

  TimeEN1545 GetTimeEN1545Value(void) const;

  TimePTools GetTimeValue(void) const;

  ShortTimeEN1545 GetShortTimeEN1545Value(void) const;

  ShortTime GetShortTimeValue(void) const;

  DateTimeEN1545 GetFullDateEN1545Value(void) const;

  DateTimePTools GetFullDateValue(void) const;

  void SetDate(const Date& date);

  void SetTime(const Time& time);

  void SetDateTime(const Date& date, const Time& time);

  void SetDateTime(const FullDate& date);

private:
  void PrepareData(const ByteVector& FileData);
};

#endif
