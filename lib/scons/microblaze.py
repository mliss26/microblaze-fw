################################################################################
# SCons MicroBlaze Tools
#
# Copyright (c) 2022 Matt Liss
# BSD-3-Clause
################################################################################
import os
from SCons.Script import *

def exists(env):
    exist = True
    #print('{}.exists(): {}'.format(os.path.basename(__file__).rstrip('.py'), exist))
    return exist

def generate(env):
    '''Add Builders and construction variables to the environment.'''
    #print('{}.generate()'.format(os.path.basename(__file__).rstrip('.py')))

    if 'default' not in env['TOOLS']:
        env.Tool('default')

    if 'utils' not in env['TOOLS']:
        env.Tool('utils')

    # setup microblaze tools
    env.Replace(CC = 'mb-gcc')
    env.Replace(CXX = 'mb-g++')
    env.Replace(AS = 'mb-gcc')
    env.Replace(ASCOM = '$AS $CFLAGS $CCFLAGS $_CCCOMCOM -x assembler-with-cpp -o $TARGET -c $SOURCES')
    env.Replace(CPP = 'mb-cpp')
    env.Replace(AR = 'mb-ar')
    env.Replace(LINK = 'mb-gcc')
    env.Replace(RANLIB = 'mb-ranlib')
    env.Replace(OBJCOPY = 'mb-objcopy')
    env.Replace(SIZE = 'mb-size')
    env.Replace(GDB = 'mb-gdb')
    env.Replace(LIBGEN = 'libgen')
    env.Replace(APPGURU = 'appguru')

    env.Append(CCFLAGS = [
            '-mcpu=${MCPU}',
            '-mlittle-endian',
            '-mxl-soft-mul',
            '-Wl,--no-relax',
            '-fmessage-length=0',
            '-std=c99',
            '-mno-xl-reorder',
            '-ffunction-sections',
            '-fdata-sections',
            '-Wall',
    ])
    env.Append(LINKFLAGS = [
            '-Wl,-T',
            '-Wl,${LDSCRIPT}',
            '-mcpu=${MCPU}',
            '-mlittle-endian',
            '-mxl-soft-mul',
            '-Wl,--no-relax',
            '-Wl,-gc-sections'
    ])
    env.Append(CPPPATH = [
            '#lib/include',
    ])

    # default processor options
    env.Append(MCPU = 'v8.40.a')
    env.Append(PROCESSOR = 'microblaze_mcs_v1_4')

    env.AddMethod(microblaze_bsp, 'MicroBlazeBSP')


def microblaze_bsp(env, target, source):
    '''Build a MicroBlaze BSP from a system description.'''

    env.Append(UBLAZE_BSP = target)
    env.Append(CPPPATH = [ '${UBLAZE_BSP}/${PROCESSOR}/include' ])
    env.Append(LIBPATH = [ '${UBLAZE_BSP}/${PROCESSOR}/lib' ])
    env.Append(LIBS = [ 'xil' ])

    # use the generated linker script if one isn't provided by user
    if ('LDSCRIPT' not in env):
        env.Append(LDSCRIPT = '${UBLAZE_BSP}/lscript.ld')

    # build the MSS from provided system XML
    mss = env.Command('${UBLAZE_BSP}/system.mss', source,
            ['${APPGURU} -hw $SOURCES -app empty_application -od ${UBLAZE_BSP}',
             'rm -f ${UBLAZE_BSP}/Makefile ${UBLAZE_BSP}/README.txt'])
    env.NoClean(mss)

    bsp_targets = [
        '${UBLAZE_BSP}/${PROCESSOR}/include/xparameters.h',
        '${UBLAZE_BSP}/${PROCESSOR}/lib/libc.a',
        '${UBLAZE_BSP}/${PROCESSOR}/lib/libm.a',
        '${UBLAZE_BSP}/${PROCESSOR}/lib/libxil.a',
    ]
    env.NoClean(bsp_targets)

    env.Command(bsp_targets, [source, mss],
            ['${LIBGEN} -pe ${PROCESSOR} -od ${UBLAZE_BSP} -hw ${SOURCES}',
             'rm -f ${UBLAZE_BSP}/libgen.log'])

    return env.Alias('bsp', bsp_targets)
