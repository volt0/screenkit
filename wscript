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

    if sys.platform.startswith('win32'):
        conf.load('msvc')
        
        conf.env.append_value('INCLUDES', 'extras')
        conf.env.append_value('LIBPATH', 'extras')

        conf.env.append_value('INCLUDES', sysconfig.get_python_inc(True))
        conf.env.append_value('LIBPATH', os.path.join(sys.prefix, 'libs'))

        conf.env.LIB.extend([
            'kernel32',
            'user32',
            'gdi32',
            'advapi32',
            'shell32',
            'opengl32',
            'glu32',
            'glew32',
            ])

    conf.check_cc(lib=('python%s' % sysconfig.get_config_vars('VERSION')[0]))
    conf.check(header_name='Python.h')
    conf.check(header_name='gl/glew.h')
    
    # conf.env.append_value('CFLAGS', '-O2')

    for i in conf.env.LIB:
        conf.check_cc(lib=i)

    # conf.write_config_header('config.h')

def build(bld):
    bld(
        target   = 'screen',
        features = 'c cshlib pyshlib',
        source   = bld.path.ant_glob([
            'src/*.c',
            'assets/*.hex',
            ]),
        lib      = bld.env.LIB,
    )

    # bld(
    #     target   = 'test',
    #     features = 'c cprogram',
    #     source   = bld.path.ant_glob(['assets/*.c', 'test/*.c']),
    #     lib      = ['glew32', 'glut32', 'opengl32'],
    #     # use      = ['screen'],
    # )

import re
from waflib.TaskGen import feature, before_method, after_method, extension
from waflib.Task import Task

@feature('pyshlib')
@after_method('apply_bundle')
@before_method('propagate_uselib_vars', 'apply_link')
def pyshlib_setenv(self):
    self.env.cshlib_PATTERN = '%s.pyd'
    self.env.cxxshlib_PATTERN = '%s.pyd'

@feature('hexfont')
@extension('.hex')
def hexfont_process(self, in_node):
    out_node = in_node.change_ext('.c')
    self.create_task('hexfont', in_node, out_node)
    self.source.append(out_node)

class hexfont(Task):
    color = 'CYAN'
    def run(self):
        font_name = self.inputs[0].name.split(os.extsep, 1)[0]

        pattern_blank = [
            '00542A542A542A542A542A542A542A00',
            'FFB9C5EDD5D5D5D5D5D5D5D5EDB991FF',
            ]

        glyph_re = re.compile('([0-9A-f]{4}):([0-9A-f]{32,64})')

        def glyph_process(s):
            match = glyph_re.match(s)
            return (int(match.group(1), 16), match.group(2))

        glyphs = dict(map(
            glyph_process,
            self.inputs[0].read().splitlines(True)
            ))

        glyph_width = 8
        glyph_height = 16

        info = {}
        result = '#include <stdint.h>\n'

        bmp = bitmap_renderer()
        
        # Sysytem glyphs

        # Basic Latin
        for i in range(0x0020, 0x007f):
            bmp.render(glyphs[i])


        result += 'static const int8_t %sBitmap[] = {\n' % font_name
        result += bmp.done()
        result += '0x00};\n'

        # for i in glyphs:
        #     result += '//{0x%x, "%s"}\n' % (i['id'], i['bitmap'])

        # self.inputs[0].__str__()
        self.outputs[0].write(result);
        return 0

class bitmap_renderer:
    texture_width = 256

    def __init__(self):
        self.bitmap_rows = [
            '', '', '', '',
            '', '', '', '',
            '', '', '', '',
            '', '', '', '',
            ]
        self.row = 0
        self.col = 0
        self.result = ''

    def render(self, ch):
        for i in range(0, 16):
            self.bitmap_rows[i] += '0x%s,' % ch[(i*2):(i*2)+2]
        
        self.row += 1
        if self.row >= self.texture_width:
            self.col += 1
            self.row = 0 

    def done(self):
        for i in self.bitmap_rows:
            self.result += '%s\n' % i

        return self.result


    # def render_glyph2(ch):
    #     if len(ch) == 32:
    #         off = 0
    #         for i in range(0, 15):
    #             bitmap_rows[i] += '0x%s,' % ch[off:off+2]
    #             off += 2
    #             row += 1
    #     else:
    #         off = 0
    #         for i in range(0, 15):
    #             bitmap_rows[i] += '0x%s,0x%s' % (ch[off:off+2], ch[off+2:off+4])
    #             off += 4
    #             row += 2
