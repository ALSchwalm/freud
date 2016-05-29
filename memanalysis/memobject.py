import struct
from collections import namedtuple


class MemoryObjectMeta(type):
    def __new__(cls, name, bases, attrs):
        if "fields_desc" not in attrs:
            for base in bases:
                if hasattr(base, "fields_desc"):
                    attrs["fields_desc"] = base.fields_desc
                    break

        combined_fmt = "".join([f.fmt for f in attrs["fields_desc"]])

        new = super(MemoryObjectMeta, cls).__new__(cls, name, bases, attrs)
        new.struct = struct.Struct(combined_fmt)
        new.FieldType = namedtuple("Fields",
                                   [f.name for f in attrs["fields_desc"]])
        return new


class MemoryObject(object):
    __metaclass__ = MemoryObjectMeta
    fields_desc = []

    @classmethod
    def from_bytes(cls, s):
        if len(s) < cls.struct.size:
            return None
        else:
            out = cls()
            out.fields = cls.FieldType._make(cls.struct.unpack(s[:cls.struct.size]))

            # namedtuple is immutable (but fast), so use a separate dict
            # for any fields that have different representations
            out.alt_fields = {}

            for i, bs in enumerate(out.fields):
                val = bs
                if hasattr(cls.fields_desc[i], "from_bytes"):
                    val = cls.fields_desc[i].from_bytes(bs)
                    out.alt_fields[cls.fields_desc[i].name] = val
                if not cls.fields_desc[i].verify(val):
                    return None
            return out
        return True

    def show(self):
        msg = "###[ {} ]###\n".format(self.__class__.__name__)
        width = max(len(f.name) for f in self.__class__.fields_desc)
        field_format = "  {:<" + str(width) + "} = {}\n"
        for i, f in enumerate(self.__class__.fields_desc):
            msg += field_format.format(f.name, f.format(getattr(self, f.name)))
        print(msg.strip())

    def __getattr__(self, attr):
        if attr in self.alt_fields:
            return self.alt_fields[attr]
        elif hasattr(self.fields, attr):
            return getattr(self.fields, attr)
        raise AttributeError(attr)
