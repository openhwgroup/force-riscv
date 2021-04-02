===============================
file_read_backwards
===============================


.. image:: https://img.shields.io/pypi/v/file_read_backwards.svg
        :target: https://pypi.python.org/pypi/file_read_backwards

.. image:: https://img.shields.io/travis/RobinNil/file_read_backwards.svg?branch=master
        :target: https://travis-ci.org/RobinNil/file_read_backwards.svg?branch=master

.. image:: https://readthedocs.org/projects/file-read-backwards/badge/?version=latest
        :target: https://file-read-backwards.readthedocs.io/en/latest/?badge=latest
        :alt: Documentation Status

.. image:: https://pyup.io/repos/github/RobinNil/file_read_backwards/shield.svg
     :target: https://pyup.io/repos/github/RobinNil/file_read_backwards/
     :alt: Updates


Memory efficient way of reading files line-by-line from the end of file


* Free software: MIT license
* Documentation: https://file-read-backwards.readthedocs.io.


Features
--------

This package is for reading file backward line by line as unicode in a memory efficient manner for both Python 2.7 and Python 3.

It currently supports ascii, latin-1, and utf-8 encodings.

It supports "\\r", "\\r\\n", and "\\n" as new lines.

Usage Examples
--------------

An example of using `file_read_backwards` for `python2.7`::

    #!/usr/bin/env python2.7

    from file_read_backwards import FileReadBackwards

    with FileReadBackwards("/tmp/file", encoding="utf-8") as frb:

        # getting lines by lines starting from the last line up
        for l in frb:
            print l

Another example using `python3.3`::

    from file_read_backwards import FileReadBackwards

    with FileReadBackwards("/tmp/file", encoding="utf-8") as frb:

        # getting lines by lines starting from the last line up
        for l in frb:
            print(l)


Another way to consume the file is via `readline()`, in `python3.3`::

    from file_read_backwards import FileReadBackwards

    with FileReadBackwards("/tmp/file", encoding="utf-8") as frb:

        while True:
            l = frb.readline()
            if not l:
                break
            print(l, end="")

Credits
---------

This package was created with Cookiecutter_ and the `audreyr/cookiecutter-pypackage`_ project template.

.. _Cookiecutter: https://github.com/audreyr/cookiecutter
.. _`audreyr/cookiecutter-pypackage`: https://github.com/audreyr/cookiecutter-pypackage

