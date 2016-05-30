import sys
from freud.memcontext import *
from freud.memobject import *
from freud.fields import *

# From https://github.com/openssl/openssl/blob/OpenSSL_1_0_2-stable/ssl/ssl.h#L498
class OpenSSL_Session(MemoryObject):
    fields_desc = [
        BitMask(UnsignedInt, "ssl_version", {
            0x300 : "SSL Version 3",
            0x003 : "Major Version 3"
        }),
        UnsignedInt("key_arg_length"),
        Array(Byte, "key_arg", 8),
        Value(Int, "master_key_length", 48),
        Array(Byte, "master_key", 48),
        Value(UnsignedInt, "session_id_length", 32),
        Array(Byte, "session_id", 32)
    ]


class BoringSSL_Session(MemoryObject):
    fields_desc = [
        UnsignedInt("references"),
        BitMask(UnsignedInt, "ssl_version", {
            0x300 : "SSL Version 3",
            0x003 : "Major Version 3"
        }),
        UnsignedInt("key_exchange_info"),
        Value(Int, "master_key_length", 48),
        Array(Byte, "master_key", 48),
        Value(UnsignedInt, "session_id_length", 32),
        Array(Byte, "session_id", 32)
    ]


m = MemoryContext.from_linux_pid(sys.argv[1])
with open("keys.out", "w") as f:
    for addr, s in m.find_all(BoringSSL_Session):
        s.show()
        f.write("RSA Session-ID:{} Master-Key:{}\n".format(s.session_id.encode("hex"),
                                                           s.master_key.encode("hex")))
