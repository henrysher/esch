#!/usr/bin/env python
import sys
import re
import os
import glob

def parse_file(filename):
    fd = open(filename)
    pat = re.compile('U\\+([0-9A-Fa-f]*)[\t ][\t ]*')

    line = fd.readline()
    m = pat.match(line)
    valuestr = m.group(1)
    sequence_list = []
    sequence = []
    previous_val = 0
    current_val = int(valuestr, 16)
    sequence = [ current_val ]
    line = fd.readline()
    while line:
        previous_val = current_val
        m = pat.match(line)
        valuestr = m.group(1)
        current_val = int(valuestr, 16)
        if current_val == previous_val + 1:
            sequence.append(current_val)
        else:
            sequence_list.append(sequence)
            sequence = [ current_val ]
        line = fd.readline()
    sequence_list.append(sequence)

    category = os.path.basename(filename).split('.')[0]
    up_bounds = []
    low_bounds = []
    for each_sequence in sequence_list:
        low_bounds.append(each_sequence[0])
        up_bounds.append(each_sequence[-1])

    print("#include \"esch.h\"")
    print('static int\nesch_unicode_range_%s_low_bound[] = ' % \
            category)
    print('{')
    for each_low_bound in low_bounds:
        print('    0x%x, ' % each_low_bound)
    print('};')
    print('static int\nesch_unicode_range_%s_up_bound[] = ' % \
            category)
    print('{')
    for each_up_bound in up_bounds:
        print('    0x%x, ' % each_up_bound)
    print('};')

    print('int\nesch_unicode_is_range_%s(esch_unicode ch)' % category)
    print('{')
    print('    size_t n = 0;')
    print('    for (; n < sizeof(esch_unicode_range_%s_up_bound) / sizeof(esch_unicode); ++n)' %\
                        category)
    print('    {')
    print('        if (ch >= esch_unicode_range_%s_low_bound[n] && ch <= esch_unicode_range_%s_up_bound[n])' % \
            (category, category))
    print('            return 1;')
    print('    }')
    print('    return 0;')
    print('}')
    fd.close()

if __name__ == '__main__':
    print('/* DON\'T MODIFY: The code below is automatically generated. */')
    print('/* Data source: http://www.fileformat.info/info/unicode/category/index.htm */')
    if (os.path.isdir(sys.argv[1])):
        filelist = glob.glob(os.path.join(sys.argv[1], '*.txt'))
        filelist.sort()
        for eachfile in filelist:
            parse_file(eachfile)
    else:
        parse_file(sys.argv[1])

