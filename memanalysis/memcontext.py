from collections import namedtuple
import re

MemoryRegion = namedtuple("MemoryRegion",
                          ["range", "permissions", "offset",
                           "device", "inode", "path"])


def parse_memory_entry(line):
    m = re.match(r"(\S+)-(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)?",
                 line)
    return MemoryRegion(
        range=(int(m.group(1), 16), int(m.group(2), 16)),
        permissions=m.group(3),
        offset=int(m.group(4), 16),
        device=m.group(5),
        inode=int(m.group(6)),
        path=m.group(7),
    )


class MemoryContext(object):
    def __init__(self):
        self.regions = {}

    @classmethod
    def from_linux_pid(cls, pid):
        ctx = cls()
        ctx.linux_pid = pid

        with open("/proc/{}/maps".format(pid), "r") as maps:
            with open("/proc/{}/mem".format(pid), "rb") as mem:
                for line in maps.readlines():
                    try:
                        entry = parse_memory_entry(line)
                        mem.seek(entry.range[0])
                        contents = mem.read(entry.range[1] - entry.range[0])
                    except:
                        continue
                    ctx.register_region(contents, entry.range[0],
                                        entry.range[1], entry.path)
        return ctx

    def register_region(self, region, start_addr, end_addr, name=None):
        self.regions[(start_addr, end_addr, name)] = region

    def find_all(self, mem_object, alignment=16):
        for bounds, region in self.regions.iteritems():
            for i in range(0, bounds[1] - bounds[0], alignment):
                out = mem_object.from_bytes(region[i:i+mem_object.struct.size],
                                            self)
                if out is not None:
                    yield bounds[0] + i, out

    def is_valid_address(self, value, region_name=None):
        for info in self.regions:
            if region_name is not None and info[2] != region_name:
                continue
            else:
                if value >= info[0] and value <= info[1]:
                    return True
        return False
