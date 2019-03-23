from building import *

cwd = GetCurrentDir()

Import('asenv')
ASROOT = asenv['ASROOT']

def IsNeedBuild(obj):
    if(GetOption('force')):
        return True
    robj = 'build/any/%s.o'%(os.path.basename(obj)[:-2])
    if(os.path.exists(robj)):
        rtm = os.path.getmtime(robj)
        stm = os.path.getmtime(obj)
        if(stm < rtm):
            return False
    return True

def Program9S12(target, objs, env):
    if(GetOption('clean')):
        RunCommand('rm -fv build/any/*.o')
        exit()
    cw = os.getenv('CWCC_PATH')
    if(cw is None):
        cw = 'C:/Program Files (x86)/Freescale/CodeWarrior for S12(X) V5.0'
    if(not os.path.exists(cw)):
        print('==> Please set environment CWCC_PATH\n\tset CWCC_PATH=/path/to/codewarrior_compiler')
        exit()
    HIWAVE ='"%s/Prog/hiwave.exe" '%(cw)
    if('run' in COMMAND_LINE_TARGETS):
        cmd = HIWAVE+'-W -Prod=TBDML.ini -instance=tbdml '+os.path.abspath('any.abs')+' -CMD="Go"'
        if('sim' in COMMAND_LINE_TARGETS):
            cmd = HIWAVE+'-W -Prod=Full_Chip_Simulation.ini -instance=sim '+os.path.abspath('any.abs')
        print('cmd is:', cmd)
        with open('run9s12.bat','w') as fp:
            fp.write('@echo off\ncd %s/Project\n%s\n'%(cwd,cmd))
        if(0 != os.system('run9s12')):
            print('run of %s failed\n  %s\n'%(target, cmd))
        exit()
    CC   = cw + '/Prog/chc12.exe'
    AS   = cw + '/Prog/ahc12.exe'
    LINK = cw + '/Prog/linker.exe'
    MAKE = '"{0}/Prog/piper.exe" "{0}/Prog/maker.exe"'.format(cw)
    with open('.makefile.9s12','w') as fp:
        fp.write('CC = "%s"\n'%(CC))
        fp.write('AS = "%s"\n'%(AS))
        fp.write('LD = "%s"\n'%(LINK))
        fp.write('COMMON_FLAGS = -WErrFileOff -WOutFileOff -EnvOBJPATH=build/any\n')
        fp.write('C_FLAGS   = -I"%s/lib/hc12c/include" -Mb -CpuHCS12X\n'%(cw))
        fp.write('ASM_FLAGS = -I"%s/lib/hc12c/include" -Mb -CpuHCS12X\n'%(cw))
        fp.write('LD_FLAGS  = -M -WmsgNu=abcet\n')
        fp.write('LIBS = "%s/lib/hc12c/lib/ansixbi.lib"\n'%(cw))
        fp.write('INC = ')
        for p in env['CPPPATH']:
            fp.write('-I%s '%(p))
        fp.write('\nC_FLAGS += ')
        for d in env['CPPDEFINES']:
            fp.write('-D%s '%(d))
        for m in env['MODULES']:
            fp.write('-DUSE_%s=1 '%(m))
        fp.write('\n\nOBJS = ')
        for obj in objs:
            obj = str(obj)
            if(obj.endswith('.c') or obj.endswith('.C')):
                if(IsNeedBuild(obj)):
                    fp.write(obj[:-2]+'.o ')
        fp.write('\n\nOBJS_LINK = ')
        for obj in objs:
            obj = str(obj)
            if(obj.endswith('.c') or obj.endswith('.C')):
                fp.write('build/any/%s.o '%(os.path.basename(obj)[:-2]))
        fp.write('''\n
.asm.o:
    $(ASM) $*.asm $(COMMON_FLAGS) $(ASM_FLAGS)

.c.o:
    $(CC) $*.c $(INC) $(COMMON_FLAGS) $(C_FLAGS)

all:$(OBJS) {0}.abs
    echo "done all"

{0}.abs :
    $(LD) {1} $(COMMON_FLAGS) $(LD_FLAGS) -Add($(OBJS_LINK)) -Add($(LIBS)) -M -O$*.abs'''.format(
        target, env['LINK_SCRIPTS'])
    )
    cmd = '%s .makefile.9s12'%(MAKE)
    with open('build9s12.bat','w') as fp:
        fp.write('@echo off\n%s\n'%(cmd))
    if(0 != os.system('build9s12')):
        print('build of %s failed\n  %s\n'%(target, cmd))

asenv['Program'] = Program9S12

MODULES = ['ECUM','SCHM','MCU',
           'DET','DEM',
           'ASKAR','SHELL','CLIB_STRTOK_R',
           'PORT','DIO','ADC',
           'CAN','CANIF','PDUR','COM','COMM','CANTP',
           'DCM','CANNM','CANSM','NM','OSEKNM','XCP',
           'FLS','FEE','MEMIF','NVM','CLIB_MBOX',
           'MPC9S12XEP100'
           ]
asenv['LINK_SCRIPTS'] = '%s/Project/prm/Project.prm'%(cwd)

# post special process for asboot release
if(asenv['RELEASE']=='asboot'):
    MODULES = ['MCU','DET',
               'CAN','CANTP','DCM',
               'FLASH','MCU',
               'ANYOS',
               'MINIBLT',
               'CANTP_MINI','DCM_MINI',
               'CLIB_STDIO_PRINTF',
               'SHELL','RINGBUFFER','CLIB_STRTOK_R',
               'MPC9S12XEP100'
           ]
    asenv.Append(CPPDEFINES=['CMDLINE_MAX=256','FLASH_CMD_MAX_DATA=512'])

ARCH='none'
arch='9s12x'

if(asenv['MODULES'] is None):
    asenv['MODULES'] = MODULES
else:
    MODULES = asenv['MODULES']

asenv['ARCH'] = ARCH
asenv['arch'] = arch

objs = []

if(asenv['RELEASE']=='asboot'):
    objs += Glob('miniblt/Sources/*.c')
    asenv.Append(CPPPATH=['%s/miniblt/Sources'%(cwd)])
else:
    objs += Glob('Project/Sources/*.c')
    asenv.Append(CPPPATH=['%s/Project/Sources'%(cwd)])
asenv.Append(CPPPATH=['%s/../../board.posix/common'%(cwd)])
asenv.Append(CPPDEFINES=['IS_ARCH16',
                         '_G_va_list=va_list',
                         'DCM_DEFAULT_RXBUF_SIZE=512',
                         'DCM_DEFAULT_TXBUF_SIZE=512'])

if('MCU' in MODULES):
    objs += Glob('mcal/Mcu.c')

if('CAN' in MODULES):
    objs += Glob('mcal/Can.c')

if('SHELL' in MODULES):
    asenv.Append(CPPDEFINES=['USE_SHELL_WITHOUT_TASK','ENABLE_SHELL_ECHO_BACK'])

Return('objs')
