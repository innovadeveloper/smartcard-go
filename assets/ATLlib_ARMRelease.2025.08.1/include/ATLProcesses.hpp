#ifndef ATL_PROCESSES_HPP
#define ATL_PROCESSES_HPP

#include "IOData.hpp"
#include "Utils/types.hpp"

namespace atl
{
  class ATLProcesses
  {

  public:
    /**
     * Reads and establish the config values needed through JSON files defined.
     *
     * Parameter:
     *  - path (in). Optional value. Refers the location of json files
     *
     * Return: Process result code
     */
    static ProcessResultCode SetConfiguration(const std::string& Path = "");

    /**
     * Startup of the system.
     *  - Initializes the SAM card by authentication.
     *  - Activates offline crypto
     *  - Sets the global values
     *
     * Parameter:
     *  - AuthKey (in) -> Authentication key value
     *  - KeyNo (in) -> Authentication key index in SAM
     *  - KeyVer (in) -> Authentication key version
     */
    static ProcessResultCode Initialization(const ByteVector& AuthKey, const Byte KeyNo, const Byte KeyVer);

    /**
     * Tears down the system
     *  - Kills authentication in SAM
     *
     * Return:
     *  - `true` if session is correctly closed on SAM. `false` if something fails
     */
    static bool CloseSession(void);

    /**
     * Validation of a card.
     * Precondition:
     *  - Have been initialized
     *
     * Parameter:
     *  - `InputData` (in) . `InputValidationData` needed to
     *  - `OutputData` (out). XML generated records
     *  - `UIMessage` (out) . Message to show in the user screen
     *
     * Return:
     *  - `ProcessResultCode`
     */
    static ProcessResultCode ValidateCard(const InputValidationData& InputData, std::string& OutputData,
                                          UIMessageAndSignal& UIData);

    /**
     * Read the content of a ATL CBT card.
     * Precondition:
     *  - Have been initialized
     *
     * Parameter:
     *  - `OutputData` (out). JSON string with read data
     *
     * Return:
     *  - `ProcessResultCode`
     */
    static ProcessResultCode ReadFullCBTCard(std::string& OutputData);
  };
} // namespace atl
#endif // ATL_PROCESSES_HPP
