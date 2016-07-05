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

    static void before_check() { usleep(1); }
};

int main(int argc, char** argv) {
    std::ofstream out("keys.log");
    std::set<freud::address_t> known_matches;

    freud::MemoryContext ctx(atoi(argv[1]), true);

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
                << "\n";
            known_matches.insert(iter.address());
        }
    }
}
