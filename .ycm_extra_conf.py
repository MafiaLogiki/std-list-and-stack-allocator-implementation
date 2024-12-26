#!/usr/bin/python3

import os
import ycm_core

def Settings(**kwargs):
    return {
        'flags': ['-x', 'c++', '-std=c++20', '-Wall', '-Wextra', '-pedantic', '-Weffc++']
    }
