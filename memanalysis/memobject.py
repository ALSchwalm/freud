import struct
from collections import namedtuple


class MemoryObjectMeta(type):
    def __new__(cls, name, bases, attrs):
        if "fields_desc" not in attrs:
            for base in bases:
                if hasattr(base, "fields_desc"):
                    attrs["fields_desc"] = base.fields_desc
                    break

        combined_fmt = ""
        for f in attrs["fields_desc"]:
            if isinstance(f, MemoryObject):
                combined_fmt += str(struct.calcsize(f.fmt)) + "s"
            else:
                combined_fmt += f.fmt
        new = super(MemoryObjectMeta, cls).__new__(cls, name, bases, attrs)
        new.fmt = combined_fmt
        new.struct = struct.Struct(combined_fmt)
        new.FieldType = namedtuple("Fields",
                                   [f.name for f in attrs["fields_desc"]])
        return new


class MemoryObject(object):
    __metaclass__ = MemoryObjectMeta
    fields_desc = []

    def __init__(self, name=""):
        self.name = name

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
                if isinstance(cls.fields_desc[i], MemoryObject):
                    subobj = cls.fields_desc[i].__class__.from_bytes(bs)
                    if subobj is None:
                        return None
                    subobj.name = cls.fields_desc[i].name
                    out.alt_fields[cls.fields_desc[i].name] = subobj
                else:
                    if hasattr(cls.fields_desc[i], "from_bytes"):
                        val = cls.fields_desc[i].from_bytes(bs)
                        out.alt_fields[cls.fields_desc[i].name] = val
                    if not cls.fields_desc[i].verify(val):
                        return None
            return out
        return True

    def format(self, level=0):
        prefix = (2 * level * " ")
        if self.name == "":
            msg = "###[ {} ]###\n".format(self.__class__.__name__)
        else:
            msg = prefix + "{} = \n".format(self.name)
        width = max(len(f.name) for f in self.__class__.fields_desc)
        field_format = prefix + "  {:<" + str(width) + "} = {}\n"
        for i, f in enumerate(self.__class__.fields_desc):
            if isinstance(f, MemoryObject):
                msg += getattr(self, f.name).format(level=level+1)
            else:
                msg += field_format.format(f.name, f.format(getattr(self, f.name)))
        return msg

    def show(self):
        print(self.format())

    def __getattr__(self, attr):
        if "alt_fields" in self.__dict__ and attr in self.__dict__["alt_fields"]:
            return self.__dict__["alt_fields"][attr]
        elif "fields" in self.__dict__ and hasattr(self.fields, attr):
            return getattr(self.fields, attr)
        raise AttributeError(attr)
