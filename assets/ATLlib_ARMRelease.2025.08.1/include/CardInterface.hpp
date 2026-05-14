/**
 * This is the interface for a reader in NXP library. User must define static methods
 * `PICCExchange` and `SAMEXchange` as follow in this header
 */

#ifndef CARDINTERFACE_HPP
#define CARDINTERFACE_HPP

#include <vector>
#include <functional>

namespace nxp {

using ByteVector = std::vector<unsigned char>;
using ExchangeCallback = std::function<void(const ByteVector&, ByteVector&)>;

class CardInterface {
public:
    static void PICCExchange(const ByteVector& request, ByteVector& response);
    static void SAMExchange(const ByteVector& request, ByteVector& response);

    static void SetPICCExchangeCallback(ExchangeCallback cb);
    static void SetSAMExchangeCallback(ExchangeCallback cb);
};

} // namespace nxp

#endif
