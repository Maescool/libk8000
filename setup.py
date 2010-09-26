#!/usr/bin/env python
#
# k8000 install script
#
# See COPYING for info about the license (GNU GPL)
# Check AUTHORS to see who wrote this software.

from distutils.core import setup
from distutils.extension import Extension
from pyk8000 import VERSION
import sys, glob, re, os

# Check for Python < 2.2
if sys.version < '2.2':
    sys.exit('Error: Python-2.2 or newer is required. Current version:\n %s'
             % sys.version)

authors = [ ('Pieter Maes', 'maescool@gmail.com') ]

lname = max([len(author[0]) for author in authors])
__author__ = '\n'.join(['%s <%s>' % (author[0].ljust(lname), author[1])
                        for author in authors])

short = 'Connection library to Velleman Kit K8000'
long = '''\
This is a connection Library to the Velleman K8000 Input/Output interface card.'''

classifiers = [
    'Development Status :: 5 - Production/Stable',
    'Environment :: Console',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: GNU General Public License (GPL)',
    'Natural Language :: English',
    'Operating System :: POSIX :: Linux',
    'Programming Language :: Python',
    'Topic :: Input/outpur :: Driver' ]
py_version='python%d.%d' % (sys.version_info[0],sys.version_info[1])
incl_dir = [ os.path.join(sys.prefix,'include',py_version), os.curdir ]

setup(name='pyk8000',
      version=VERSION,
      description=short,
      long_description=long,
      classifiers=classifiers,
      author=', '.join([author[0] for author in authors]),
      author_email=', '.join([author[1] for author in authors]),
      url='http://github.com/Maescool/libk8000',
      ext_modules = [Extension('_k8000', sources=['k8000.c','k8000.i'],swig_opts=['-shadow'])],
      py_modules = ["k8000"],
      license='GPL'
)
