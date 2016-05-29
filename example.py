import sys
import mmap
from memanalysis.memobject import *
from memanalysis.fields import *

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
    ]

with open(sys.argv[1], "rb") as f:
    contents = mmap.mmap(f.fileno(), 0, prot=mmap.PROT_READ)
    for i in range(0, contents.size(), 16):
        out = SSL_Session.from_bytes(contents[i:i+SSL_Session.struct.size])
        if out:
            out.show()
