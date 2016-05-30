
class MemoryContext(object):
    def __init__(self):
        self.regions = {}

    def register_region(self, region, start_addr, end_addr):
        self.regions[(start_addr, end_addr)] = region

    def find_all(self, mem_object, alignment=16):
        for bounds, region in self.regions.iteritems():
            for i in range(0, bounds[1] - bounds[0], alignment):
                out = mem_object.from_bytes(region[i:i+mem_object.struct.size])
                if out is not None:
                    yield bounds[0] + i, out
