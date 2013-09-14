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

    print('int\nesch_unicode_is_range_%s(esch_unicode ch)' % \
            os.path.splitext(os.path.basename(filename))[0])
    print('{')
    first_sequence = sequence_list[0]
    print('    if (ch >= 0x%x && ch <= 0x%x) { return 1; }' % \
            (first_sequence[0], first_sequence[-1]))
    for each_sequence in sequence_list[1:]:
        print('    else if (ch >= 0x%x && ch <= 0x%x) { return 1; }' % \
                (each_sequence[0], each_sequence[-1]))
    print('    else { return 0; }')
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

