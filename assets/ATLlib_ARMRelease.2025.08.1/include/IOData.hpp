#ifndef IODATA_HPP
#define IODATA_HPP

#include "Utils/types.hpp"
#include <stdexcept>

/**
 * Needed data for each validation
 */
struct InputValidationData
{
  /**
   * Date and time
   */
  Date currentDate;
  Time currentTime;

  /* *
   * Terminal placement type. Station or onboard.
   */
  bool terminalPlacementType;

  /* *
   * System type. Open or closed.
   */
  bool systemType;

  /* *
   * Destination indicated at entry.
   */
  bool indicatedDestination;

  /* *
   * Type of Validation. Entry (1) or exit (2).
   */
  Byte validationType;

  /**
   * OperatorId
   */
  Word operatorId;

  /**
   * Line Id
   */
  Word lineId;

  /**
   * Direction Id. Only while using and onboard terminalPlacementType. Undefined (0), outbound (1) and return (2)
   */
  Byte directionId;

  /**
   * current Station Id.
   */
  Word currentStationId;

  /**
   * Destination Station Id.
   */
  Word destinationStationId;

  /**
   * Lobby Id. When terminalPlacementType is Station.
   */
  Byte lobbyId;

  /**
   * Machine Id.
   */
  UInt equipmentId;

  /**
   * Constructor
   */
  InputValidationData(void) :
      currentDate {}, currentTime {}, terminalPlacementType {false}, systemType {false}, indicatedDestination {false},
      validationType {false}, operatorId {0U}, lineId {0U}, directionId {0U}, currentStationId {0U},
      destinationStationId {0U}, lobbyId {0U}, equipmentId {0U} {
        // do nothing
      };

  /**
   * Set date. Mandatory for any process
   */
  inline void SetCurrentDate(const Byte year, const Byte month, const Byte day)
  {
    if ((month == 0U) || (month > MAX_MONTH)) {
      throw std::out_of_range("ERROR: " + std::to_string(month) + " is not valid month number");
    }
    if ((day == 0U) || (day > MAX_DAY)) {
      throw std::out_of_range("ERROR: " + std::to_string(day) + " is not valid day number");
    }
    currentDate.year = year;
    currentDate.month = month;
    currentDate.day = day;
  };

  /**
   * Set time. Mandatory for any process
   */
  inline void SetCurrentTime(const Byte hour, const Byte min, const Byte sec)
  {
    if (hour > MAX_HOURS) {
      throw std::out_of_range("ERROR: " + std::to_string(hour) + " is not valid hours number");
    }
    if (min > MAX_MINUTES) {
      throw std::out_of_range("ERROR: " + std::to_string(min) + " is not valid minutes number");
    }
    if (min > MAX_SECONDS) {
      throw std::out_of_range("ERROR: " + std::to_string(sec) + " is not valid seconds number");
    }
    currentTime.hour = hour;
    currentTime.min = min;
    currentTime.sec = sec;
  };
};

/**
 */
struct UIMessageAndSignal
{
  /**
   * Message to show the user
   */
  std::string msgDisplay;

  /**
   * Colour to show in visual signal
   */
  std::string visualSignal;

  /**
   * Sound to reproduce
   */
  std::string acousticSignal;

  /**
   * Constructor
   */
  UIMessageAndSignal(void) : msgDisplay {""}, visualSignal {""}, acousticSignal {""}
  {
    // do nothing
  }
};

using ProcessResultCode = Word;

namespace atl
{
  namespace result_code
  {
    const ProcessResultCode UNKNOWN {0xFFF};

    const ProcessResultCode OK {0U}; // Validation completed

    const ProcessResultCode CARD_BLOCKED {2U}; // Analisis informacion, apartado 4.3

    const ProcessResultCode INVALID_APP_VERSION {4U}; // Analisis informacion, apartado 4.3
    const ProcessResultCode FAIL_AUTHENTICATION_ON_PICC {5U};
    const ProcessResultCode CARD_EXPIRED {6U}; // // Analisis informacion, apartado 4.3
    const ProcessResultCode NOT_ENOUGH_BALANCE {7U}; // Analisis informacion, apartado 4.3
    const ProcessResultCode NOT_VALID_TITLES {8U};
    const ProcessResultCode PASSBACK {9U};
    const ProcessResultCode CARD_NOT_ACTIVATED {10U}; // Analisis informacion, apartado 4.3
    const ProcessResultCode INVALID_ISSUER {11U}; // Analisis informacion, apartado 4.3
    const ProcessResultCode CARD_WITHOUT_VALID_PROFILES {12U}; // Analisis informacion, apartado 4.3
    const ProcessResultCode NOT_ENOUGH_BALANCE_FOR_PENALIZATION {13U}; // Analisis informacion, apartado 4.3
    const ProcessResultCode UNKNOWN_FARE {14U};

    const ProcessResultCode GENERIC_VALIDATION_ERROR {100U};

    const ProcessResultCode NOT_VALID_CARD_APP_FOUND {103U};

    const ProcessResultCode ERROR_READING_DATA {106U};
    const ProcessResultCode ERROR_WRITING_DATA {107U};

    const ProcessResultCode FAIL_AUTHENTICATION_ON_SAM_FOR_PICC {110U};

  } // namespace result_code
} // namespace atl

#endif // IODATA_HPP