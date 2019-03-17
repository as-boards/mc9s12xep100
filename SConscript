from building import *

cwd = GetCurrentDir()

Import('asenv')
ASROOT = asenv['ASROOT']

def Program9S12(target, objs, env):
    print('program', target)
    cw = os.getenv('CWCC_PATH')
    if(cw is None):
        cw = 'C:/Program Files (x86)/Freescale/CodeWarrior for S12(X) V5.0'
    if(not os.path.exists(cw)):
        print('==> Please set environment CWCC_PATH\n\tset CWCC_PATH=/path/to/codewarrior_compiler')
        exit()
    CC   = cw + '/Prog/chc12.exe'
    AS   = cw + '/Prog/ahc12.exe'
    LINK = cw + '/Prog/linker.exe'
    MAKE = '"{0}/Prog/piper.exe" "{0}/Prog/maker.exe"'.format(cw)
    with open('makefile.9s12','w') as fp:
        fp.write('CC = "%s"\n'%(CC))
        fp.write('AS = "%s"\n'%(AS))
        fp.write('LD = "%s"\n'%(LINK))
        fp.write('COMMON_FLAGS = -WErrFileOff -WOutFileOff -EnvOBJPATH=build/any\n')
        fp.write('C_FLAGS   = -I"%s/lib/hc12c/include" -Mb -CpuHCS12\n'%(cw))
        fp.write('ASM_FLAGS = -I"%s/lib/hc12c/include" -Mb -CpuHCS12\n'%(cw))
        fp.write('LD_FLAGS  = -M -WmsgNu=abcet\n')
        fp.write('LIBS = "%s/lib/hc12c/lib/ansibi.lib"\n'%(cw))
        fp.write('INC = ')
        for p in env['CPPPATH']:
            fp.write('-I%s '%(p))
        fp.write('\n\nOBJS = ')
        for obj in objs:
            obj = str(obj)
            if(obj.endswith('.c') or obj.endswith('.C')):
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
    $(LD) {1} $(COMMON_FLAGS) $(LD_FLAGS) -Add($(LIBS)) -Add($(OBJS_LINK)) -M -O$*.abs'''.format(
        target, env['LINK_SCRIPTS'])
    )
    cmd = '%s makefile.9s12'%(MAKE)
    print('copy and run below command\n\t%s'%(cmd))

asenv['Program'] = Program9S12

MODULES = ['ECUM','SCHM','MCU',
           'DET','DEM',
           'CLIB_STDIO_PRINTF',
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
    MODULES = ['ECUM','SCHM','MCU','DET',
               'PORT','CAN','CANIF','PDUR','CANTP','XCP',
               'DCM','FLASH','TINYOS',
               'CLIB_STDIO_PRINTF','CLIB_STRTOK_R',
               'SHELL','FLASH_CMD','MEM_CMD','JMP_CMD',
               'MPC9S12XEP100'
               ]
    asenv.Append(CPPDEFINES=['CMDLINE_MAX=4096','FLASH_CMD_MAX_DATA=2000'])

ARCH='none'
arch='9s12x'

if(asenv['MODULES'] is None):
    asenv['MODULES'] = MODULES
else:
    MODULES = asenv['MODULES']

asenv['ARCH'] = ARCH
asenv['arch'] = arch

objs = []

objs += Glob('Project/Sources/*.c')

asenv.Append(CPPPATH=['%s/Project/Sources'%(cwd)])

Return('objs')
