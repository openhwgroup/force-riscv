import sys
from io import StringIO


def dump_repr(obj, lvl=0, file=sys.stdout):
    """Outputs obj representation into a readable format.

    :param obj: Object to be displayed hierarchically
    :type obj: object
    :param lvl: Level of indention.  Mostly used internally by regressive calls.
    :type lvl: int
    :param file: file
    """
    indent = '   '
    if isinstance(obj, dict):
        print('%s{' % (lvl * indent), file=file)
        for key, val in obj.items():
            if hasattr(val, '__iter__') and not isinstance(val, str):
                print('%s%s:' % ((lvl + 1) * indent, repr(key)), file=file)
                dump_repr(val, lvl + 1, file=file)
            else:
                print('%s%s: %s,' %
                      ((lvl + 1) * indent, repr(key), repr(val)), file=file)
        print('%s},' % (lvl * indent), file=file)
    elif isinstance(obj, list):
        complex_data = False
        for val in obj:
            if hasattr(val, '__iter__'):
                complex_data = True
                break
        if complex_data:
            print('%s[' % (lvl * indent), file=file)
            for val in obj:
                if hasattr(val, '__iter__'):
                    dump_repr(val, lvl + 1, file)
                else:
                    print('%s%s,' % (
                        (lvl + 1) * indent, repr(val)), file=file)
            print('%s],' % (lvl * indent), file=file)
        else:
            print('%s%s' % ((lvl + 1) * indent, obj), file=file)
    else:
        print('%s%s,' % (lvl * indent, repr(obj)), file=file)


def dump_str(obj):
    """Return a str with output from dump_repr()

    :param obj: Object to display hierarchically.
    :type obj: object
    :return: str
    """
    with StringIO() as buf:
        dump_repr(obj, file=buf)
        return buf.getvalue()


def indices(a_list, val):
    """Always returns a list containing the indices of val in the list.

    :param a_list: List to be searched.
    :type a_list: list
    :param val: Item to be found in a_list
    :type val: object
    :return: List of integer indexes of val in a_list
    :rtype: list
    """
    return [index for index, value in enumerate(a_list) if value == val]
