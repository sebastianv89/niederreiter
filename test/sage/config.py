# Warning: this dumb parser will always use the last parameters
#          set in the config.h file

SRC_DIR = '../../src'

class Config:
    @classmethod
    def parse(cls, line):
        for s, var in cls._integers.iteritems():
            if line.startswith(s):
                setattr(cls, var, int(line[len(s):]))
        for s, var in cls._int_lists.iteritems():
            if line.startswith(s):
                setattr(cls, var, [int(x) for x in line[len(s):].strip(' \n{}').split(',')])

Config._integers = {
    '#define POLY_COUNT '  : 'poly_count',
    '#define POLY_BITS '   : 'poly_bits',
    '#define POLY_WEIGHT ' : 'poly_weight',
    '#define ERROR_WEIGHT ': 'error_weight'
}
Config._int_lists = {
    '#define THRESHOLDS '  : 'thresholds'
}

with open(SRC_DIR + '/config.h', 'r') as config_h:
    for line in config_h:
        Config.parse(line)

if __name__ == '__main__':
    print Config.poly_count
    print Config.poly_bits
    print Config.poly_weight
    print Config.error_weight
    print Config.thresholds
