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
        new.fmt = combined_fmt
        new.struct = struct.Struct(combined_fmt)
        return new


class MemoryObject(object):
    __metaclass__ = MemoryObjectMeta
    fields_desc = []

    def __init__(self, name=""):
        self.name = name
        self.fields = {}

    @classmethod
    def from_values(cls, fields):
        out = cls()
        index = 0
        for f in cls.fields_desc:
            if isinstance(f, MemoryObject):
                mem = f.from_values(fields[index:index+len(f.fields_desc)])
                if mem is None:
                    return None
                mem.name = f.name
                out.fields[f.name] = mem
                index += len(f.fields_desc)
            else:
                val = fields[index]
                if hasattr(f, "from_bytes"):
                    val = f.from_bytes(val)
                if not f.verify(val):
                    return None
                out.fields[f.name] = val
                index += 1
        return out

    @classmethod
    def from_bytes(cls, s):
        if len(s) < cls.struct.size:
            return None
        else:
            fields = cls.struct.unpack(s[:cls.struct.size])
            return cls.from_values(fields)
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
        if "fields" in self.__dict__ and attr in self.fields:
            return self.fields[attr]
        raise AttributeError(attr)
