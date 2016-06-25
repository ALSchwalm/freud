#include "freud/freud.hpp"
#include <iostream>
#include <openssl/ssl.h>

class SSL_SESSION_Matcher : public freud::MemoryObject<SSL_SESSION> {
public:
    static bool verify(const SSL_SESSION& e, uint64_t address) {
        return e.ssl_version == 771 && e.master_key_length == 48 &&
               e.session_id_length == 32;
    }
};

int main(int argc, char** argv) {
    freud::LinuxMemoryContext ctx(atoi(argv[1]), true);

    freud::MemoryContextIterator<SSL_SESSION_Matcher> iter =
        ctx.iter_matches<SSL_SESSION_Matcher>();

    for (; iter != freud::MemoryContextIterator<SSL_SESSION_Matcher>();
         ++iter) {
        std::cout << "RSA Session-ID:" << freud::format_as_hex(iter->session_id)
                  << " Master-Key:" << freud::format_as_hex(iter->master_key)
                  << std::endl;
    }
}
