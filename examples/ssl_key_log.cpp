#include "freud/freud.hpp"
#include <iostream>
#include <openssl/ssl.h>
#include <unistd.h>

class SSL_SESSION_Matcher : public freud::MemoryObject<SSL_SESSION> {
public:
    static bool verify(const freud::MemoryContext& ctx, const SSL_SESSION& e,
                       uint64_t address) {
        return e.ssl_version == 0x303 && e.master_key_length == 48 &&
               e.session_id_length == 32;
    }
};

int main(int argc, char** argv) {
    freud::MemoryContext ctx(atoi(argv[1]), true);

    freud::MemoryContextIterator<SSL_SESSION_Matcher> iter =
        ctx.scan_once<SSL_SESSION_Matcher>();

    for (; iter != ctx.end<SSL_SESSION_Matcher>(); ++iter) {
        std::cout << "RSA Session-ID:" << freud::format_as_hex(iter->session_id)
                  << " Master-Key:" << freud::format_as_hex(iter->master_key)
                  << std::endl;
    }
}