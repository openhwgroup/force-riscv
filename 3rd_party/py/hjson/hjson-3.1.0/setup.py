#!/usr/bin/env python
from __future__ import with_statement

import sys
try:
    from setuptools import setup, Command
except ImportError:
    from distutils.core import setup, Command
from distutils.errors import CCompilerError, DistutilsExecError, \
    DistutilsPlatformError

IS_PYPY = hasattr(sys, 'pypy_translation_info')
VERSION = '3.1.0'
DESCRIPTION = "Hjson, a user interface for JSON."

with open('README.rst', 'r') as f:
   LONG_DESCRIPTION = f.read()

CLASSIFIERS = filter(None, map(str.strip,
"""
Development Status :: 5 - Production/Stable
Intended Audience :: Developers
License :: OSI Approved :: MIT License
License :: OSI Approved :: Academic Free License (AFL)
Programming Language :: Python
Programming Language :: Python :: 2
Programming Language :: Python :: 2.6
Programming Language :: Python :: 2.7
Programming Language :: Python :: 3
Programming Language :: Python :: 3.3
Programming Language :: Python :: 3.4
Programming Language :: Python :: 3.5
Programming Language :: Python :: Implementation :: CPython
Programming Language :: Python :: Implementation :: PyPy
Topic :: Software Development :: Libraries :: Python Modules
""".splitlines()))

class BuildFailed(Exception):
    pass

class TestCommand(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        import sys, subprocess
        raise SystemExit(
            subprocess.call([sys.executable,
                             # Turn on deprecation warnings
                             '-Wd',
                             'hjson/tests/__init__.py']))

def run_setup():
    cmdclass = dict(test=TestCommand)
    kw = dict(cmdclass=cmdclass)

    setup(
        name="hjson",
        version=VERSION,
        description=DESCRIPTION,
        long_description=LONG_DESCRIPTION,
        keywords="json comments configuration",
        classifiers=CLASSIFIERS,
        author="Christian Zangl",
        author_email="laktak@cdak.net",
        url="http://github.com/hjson/hjson-py",
        license="MIT License",
        packages=['hjson', 'hjson.tests'],
        platforms=['any'],
        entry_points={
            "console_scripts": [
                "hjson = hjson.tool:main",
            ],
        },
        **kw,
    )

run_setup()
