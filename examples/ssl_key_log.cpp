/** ssl_key_log.cpp
 *
 * This file is a brief example that will capture and
 * record SSL session master secrets from an application
 * using OpenSSL sessions.
 *
 * Compile this program with 'g++ -I. examples/ssl_key_log.cpp -o key_log'
 *
 * The resulting 'key_log' binary can be run (as root) with:
 * './key_log <some pid>'. Where <some pid> is the PID of the
 * target process.
 *
 * While running, this program will log keys to 'keys.log' and
 * to standard output. After you have collected the keys,
 * you can decrypt some wireshark traffic by selecting:
 *
 * Edit->Preferences->Protocols->SSL
 *
 * Add adding the 'keys.log' file as the '(Pre)-Master-Secret Log Filename'
 */

#include "freud/freud.hpp"
#include <iostream>
#include <openssl/ssl.h>
#include <set>
#include <unistd.h>

class SSL_SESSION_Matcher : public freud::MemoryObject<SSL_SESSION> {
public:
    static bool verify(const freud::MemoryContext& ctx, const SSL_SESSION& e,
                       freud::address_t address) {
        bool res = e.ssl_version == 0x303 && e.master_key_length == 48 &&
                   e.session_id_length == 32;
        return res;
    }
};

int main(int argc, char** argv) {
    std::ofstream out("keys.log");
    std::set<freud::address_t> known_matches;

    freud::MemoryContext ctx(atoi(argv[1]));

    freud::MemoryContextIterator<SSL_SESSION_Matcher> iter =
        ctx.scan_forever<SSL_SESSION_Matcher>();

    for (;; ++iter) {
        if (!known_matches.count(iter.address())) {
            std::cout << "RSA Session-ID:"
                      << freud::format_as_hex(iter->session_id)
                      << " Master-Key:"
                      << freud::format_as_hex(iter->master_key) << std::endl;

            out << "RSA Session-ID:" << freud::format_as_hex(iter->session_id)
                << " Master-Key:" << freud::format_as_hex(iter->master_key)
                << std::endl;
            known_matches.insert(iter.address());
        }
    }
}
