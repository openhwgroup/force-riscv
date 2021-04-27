#!/usr/bin/env python3
"""
Compares dot-separated version strings for testing min version requirements

Usage:
    version_chk [-v | --verbose] <version_str> <required_version_str>
    version_chk -t | --test
    version_chk -h | --help

Options:
    -v --verbose    output intermediate results
    -t --test       Run self tests
    -h --help       Show this screen

"""
import re
from docopt.docopt import docopt

PASS = 0
FAIL = 1
RC={PASS: 'Passed', FAIL: 'Failed'}


def compare_versions(value, required, verbose):
    """Compares the numerical value of two dot-separated version strings

    :type value: str
    :type required: str

    """
    # allow for unexpected input for value.
    # required is within our control.
    val = [int(x) for x in re.split(r'\D+', value) if x]
    req = [int(x) for x in required.split(sep='.')]

    def log_return(result):
        if verbose:
            print('value = %14s, required = %14s, result = %s' % (val, req, RC[result]))
        return result

    for i, rev in enumerate(req):

        try:
            test_val = val[i]
        except IndexError:
            return log_return(FAIL)

        if test_val == rev:
            continue
        elif test_val < rev:
            return log_return(FAIL)
        elif test_val > rev:
            return log_return(PASS)

    return log_return(PASS)


def self_test():
    test_data = [
        [' 2.3-23f', '2.3.4'],
        [' 10.2.3', '2.3.4'],
        [' 10.2.3', '2.3.4'],
        ['2.3.5', '2.3.4'],
        ['2.3.4', ' 2.3.4'],
        ['2.3.3', '2.3.4'],
        ['2.4', '2.3.4'],
        ['2.3', '2.3.4'],
        [' 2.3.4', '2'],
        ['2.2.4', '2. 3.4'],
        ['1.3.4', '2.3.4'],
        [' 21.3.4', '2.3.4'],
        ['2.30.4', '2.3.4'],
    ]
    for v, r in test_data:
        compare_versions(v, r, True)


def main():
    arguments = docopt(__doc__, version='version_chk.py v0.9')
    verbose = arguments['--verbose']

    if arguments['--test']:
        print(arguments)
        self_test()
        exit(0)

    elif (arguments['<required_version_str>'] and arguments['<version_str>']):
        return(compare_versions(arguments['<version_str>'],
                                arguments['<required_version_str>'],
                                verbose))


if __name__ == '__main__':
    exit(main())

