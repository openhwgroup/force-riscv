=======
History
=======

1.0.0 (2016-12-18)
------------------

* First release on PyPI.

1.1.0 (2016-12-31)
------------------

* Added support for "latin-1".
* Marked the package "Production/Stable".

1.1.1 (2017-01-09)
------------------

* Updated README.rst for more clarity around encoding support and Python 2.7 and 3 support.

1.1.2 (2017-01-11)
------------------

* Documentation re-arrangement. Usage examples are now in README.rst
* Minor refactoring

1.2.0 (2017-09-01)
------------------

* Include context manager style as it provides cleaner/automatic close functionality

1.2.1 (2017-09-02)
------------------

* Made doc strings consistent to Google style and some code linting


1.2.2 (2017-11-19)
------------------

* Re-release of 1.2.1 for ease of updating pypi page for updated travis & pyup.

2.0.0 (2018-03-23)
------------------

Mimicing Python file object behavior.

* FileReadBackwards no longer creates multiple iterators (a change of behavior from 1.x.y version)
* Adding readline() function retuns one line at a time with a trailing new line and empty string when it reaches end of file.
  The fine print: the trailing new line will be `os.linesep` (rather than whichever new line type in the file).
