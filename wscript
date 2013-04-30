#!/usr/bin/env python
# encoding: utf-8

import sys
import os
from distutils import sysconfig

top = '.'
out = '.build'

VERSION='1.0.0'
APPNAME='screenkit'

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')

    conf.env.LIB = []

    glew_lib = 'glew'

    if sys.platform.startswith('win32'):
        conf.load('msvc')
        
        conf.env.append_value('INCLUDES', os.path.abspath('extras'))
        conf.env.append_value('LIBPATH', os.path.abspath('extras'))

        conf.env.append_value('INCLUDES', sysconfig.get_python_inc(True))
        conf.env.append_value('LIBPATH', os.path.join(sys.prefix, 'libs'))

        conf.env.PLATFORMSRC = 'src/win32.c'

        conf.env.LIB.extend([
            'kernel32',
            'user32',
            'gdi32',
            'advapi32',
            'shell32',
            'opengl32',
            'glu32',
            ])

        glew_lib = ['glew32', 'glew32s']
    
    
    conf.check(header_name='gl/glew.h')
    conf.check(header_name='Python.h')

    conf.check_cc(lib=glew_lib, uselib_store='GLEW')
    conf.check_cc(lib=('python%s' % sysconfig.get_config_vars('VERSION')[0]))
    
    # conf.env.append_value('CFLAGS', '-O2')

    for i in conf.env.screen_LIB:
        conf.check_cc(lib=i)

    # conf.write_config_header('config.h')

def build(bld):
    bld(
        target   ='font',
        features ='scfont c',
        assets   ='assets',
        )

    bld(
        target   = 'screen',
        features = 'c cshlib pyshlib',
        source   = bld.path.ant_glob([
            'src/screen.c',
            'src/core.c',
            bld.env.PLATFORMSRC
            ]),
        use      = 'font',
        uselib   = 'GLEW',
    )

import re
from waflib.TaskGen import feature, before_method, after_method, extension
from waflib.Task import Task

@feature('pyshlib')
@after_method('apply_bundle')
@before_method('propagate_uselib_vars', 'apply_link')
def pyshlib_setenv(self):
    self.env.cshlib_PATTERN = '%s.pyd'
    self.env.cxxshlib_PATTERN = '%s.pyd'

@feature('scfont')
@before_method('process_source')
def scfont_feature(self):
    in_folder = self.bld.srcnode.find_node(getattr(self, 'assets', None))
    out_folder = self.bld.bldnode.find_or_declare(getattr(self, 'assets', None))
    
    task = self.create_task('scfont')
    task.inputs = map(lambda x: in_folder.find_node(x), scfont.font_files)
    task.outputs = [out_folder.find_or_declare(self.target + '.c')]
    self.source = self.source or []
    self.source.extend(task.outputs)

class scfont(Task):
    color = 'CYAN'

    font_files = ['ter-u16n.bdf']

    def run(self):
        char_re = re.compile(
            'STARTCHAR.*?ENCODING ([0-9]+).*?BITMAP[\n\r]+' +
            '([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+' +
            '([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+' +
            '([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+' +
            '([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+([0-9a-fA-F]+)[\n\r]+' +
            '.*?ENDCHAR',
            re.DOTALL
            )
        
        result = '#include "../src/screen.h"\n'
        dump = self.inputs[0].read()

        renderer = scfont_renderer()
        for i in char_re.finditer(dump):
            renderer.render(list(i.groups()[1:]))

        bmp_width = renderer.texture_width
        bmp_height = renderer.row + 1

        result += 'const int fontBitmapWidthEm = %d;\n' % bmp_width
        result += 'const int fontBitmapHeightEm = %d;\n' % bmp_height
        result += 'const int fontBitmapWidthPx = %d;\n' % (bmp_width * 8)
        result += 'const int fontBitmapHeightPx = %d;\n' % (bmp_height * 16)
        result += 'const unsigned char %sBitmap[] = {\n' % 'font'#self.target
        result += renderer.done()
        result += '0x00};\n'
        
        self.outputs[0].write(result)

class scfont_renderer:
    texture_width = 128

    def __init__(self):
        self.bitmap_rows = ['' for _ in range(0, 16)]
        self.row = 0
        self.col = 0
        self.result = ''

    def render(self, ch):
        self.bitmap_rows = map(
            lambda (a,b): a + ('0x%s,' % b),
            zip(self.bitmap_rows, ch)
            )
        
        self.col += 1
        if self.col >= self.texture_width:
            self.row += 1
            self.col = 0

            self.result += ''.join(['%s\n' % i for i in self.bitmap_rows])
            self.bitmap_rows = ['' for _ in range(0, 16)]

    def done(self):
        delta = range(0, self.texture_width - self.col)
        return self.result + ''.join(['%s%s\n' % (i, ''.join(['0x00,' for _ in delta])) for i in self.bitmap_rows])
