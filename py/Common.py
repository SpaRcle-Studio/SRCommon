import os
import re
import sys
import subprocess
import typing

try:
    import numpy as np
except Exception as e:
    subprocess.run([sys.executable, 'pip', 'install', 'numpy'])
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'numpy'])
    import numpy as np

try:
    import argparse
except Exception as e:
    subprocess.run([sys.executable, 'pip', 'install', 'argparse'])
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'argparse'])
    import argparse

try:
    import xml.etree.ElementTree as ET
except Exception as e:
    subprocess.run([sys.executable, 'pip', 'install', 'elementtree'])
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'elementtree'])
    import xml.etree.ElementTree as ET

try:
    import clang
except Exception as e:
    subprocess.run([sys.executable, 'pip', 'install', 'clang'])
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'clang'])
    import clang

try:
    import hashlib
except Exception as e:
    subprocess.run([sys.executable, 'pip', 'install', 'hashlib'])
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'hashlib'])
    import hashlib

import clang.cindex
from clang.cindex import Config

import shutil
