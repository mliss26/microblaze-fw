#
# MicroBlaze MCS Firmware Project
#
# Copyright (c) 2022 Matt Liss
# BSD-3-Clause
#
import os

# setup env with microblaze tools and default build variant
env = Environment(
        ENV = {'PATH': os.environ['PATH']},
        toolpath = ['lib/scons'],
        tools = [ 'microblaze' ])
Export('env')
env.VariantDir('build/src/', 'src', duplicate=0)
env.VariantDir('build/lib/', 'lib', duplicate=0)

# build a BSP for the system
env.MicroBlazeBSP('mb-bsp', 'system.xml')

# setup build environment
env.AppendUnique(CPPPATH = [ '#src' ])

# define size of connected LCD
env.AppendUnique(CPPDEFINES = [
        'LCD_WIDTH=16',
        'LCD_HEIGHT=2',
        'LCD_INT_PUT_FUNCTIONS',
])

# build the firmware
sources = [
    'build/src/main.c',
    'build/lib/src/gcnt.c',
    'build/lib/src/hexdump.c',
    'build/lib/src/ina219.c',
    'build/lib/src/lcd.c',
    'build/lib/src/list.c',
    'build/lib/src/sdram.c',
    'build/lib/src/timer.c',
    'build/lib/src/twi.c',
]
elf = env.Program('build/microblaze-fw.elf', sources)

# integrate with parent ISE project
ise_project_dir = '..'
if (os.path.exists(ise_project_dir+'/ipcore_dir')):
    env.Alias('system', env.InstallAs('system.xml', ise_project_dir+'/ipcore_dir/microblaze_mcs_v1_4_sdk.xml'))
    env.Alias('install', env.Install(ise_project_dir+'/ipcore_dir/microblaze_mcs_v1_4/', elf))
