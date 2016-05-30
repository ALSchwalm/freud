import sys
import mmap
from memanalysis.memcontext import *
from memanalysis.memobject import *
from memanalysis.fields import *

# From https://github.com/openssl/openssl/blob/OpenSSL_1_0_2-stable/ssl/ssl.h#L498
class SSL_Session(MemoryObject):
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


with open(sys.argv[1], "rb") as f:
    contents = mmap.mmap(f.fileno(), 0, prot=mmap.PROT_READ)
    m = MemoryContext()
    m.register_region(contents,
                      int(sys.argv[2], 16),
                      int(sys.argv[3], 16))
    for addr, s in m.find_all(SSL_Session):
        print("Found SSL_Session at {}\n".format(hex(addr)))
        s.show()
        print("RSA Session-ID:{} Master-Key:{}".format(s.session_id.encode("hex"),
                                                       s.master_key.encode("hex")))
