import subprocess

def getGitHash():
    return subprocess.Popen('git rev-parse --short HEAD', stdout=subprocess.PIPE, shell=True).stdout.read().decode('utf-8').strip()

env = Environment()
env['CCFLAGS']	= '-Wall -Wextra -std=c++17 -O2 -march=native -g -DGIT_HASH=' + getGitHash()
env['CPPPATH']	= ['src', 'aicodix/dsp', 'aicodix/code', 'aicodix']

if 'WITH_X' in ARGUMENTS:
    with_x = ARGUMENTS['WITH_X']
    if with_x == '1':
        env['CCFLAGS'] += ' -DWITH_X'
        env['LIBS'] = 'X11'
    elif with_x != '0':
        print('WITH_X must be either 0 or 1')
        Exit(1)
else:
    print('WITH_X argument is mandatory')
    print('')
    print('Use WITH_X=0 to build without X Window System support')
    print('Use WITH_X=1 to build with X Window System support')
    print('')
    print('Building with X requires X11 library, but program can still run')
    print('without X available (the -n option appears).')
    Exit(1)

env.VariantDir('build', 'src', duplicate = 0)
env.AlwaysBuild(['build/version.o', 'build/ofdmtvrx'])
ofdmtvrx = env.Program('build/ofdmtvrx', Glob('build/*.cpp'))
env.Install('/usr/local/bin', ofdmtvrx)
env.Alias('install', '/usr/local/bin')
