#!/usr/bin/python

from distutils.core import setup, Extension

serek = Extension('serek',
                    sources = ['serek.c'])

setup (name = 'serek',
       version = '0.1',
       description = 'PHP serialization for Python',
       ext_modules = [serek])
