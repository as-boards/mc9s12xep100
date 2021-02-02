from building import *

cwd = GetCurrentDir()

Import('asenv')
ASROOT = asenv['ASROOT']
BDIR = asenv['BDIR']

def IsNeedBuild(obj, target):
    if(GetOption('force')):
        return True
    robj = '%s/%s/%s.o'%(BDIR, target, os.path.basename(obj)[:-2])
    if(os.path.exists(robj)):
        rtm = os.path.getmtime(robj)
        stm = os.path.getmtime(obj)
        if(stm < rtm):
            return False
    return True

def Program9S12(target, objs, env):
    target = os.path.basename(target)
    MKDir('%s/%s'%(BDIR, target))
    if(GetOption('clean')):
        RunCommand('rm -fv %s/*'%(BDIR))
        exit()
    cw = os.getenv('CWCC_PATH')
    if(cw is None):
        cw = 'C:/Program Files (x86)/Freescale/CodeWarrior for S12(X) V5.0'
    if(not os.path.exists(cw)):
        print('==> Please set environment CWCC_PATH\n\tset CWCC_PATH=/path/to/codewarrior_compiler')
        exit()
    HIWAVE ='"%s/Prog/hiwave.exe" '%(cw)
    if('run' in COMMAND_LINE_TARGETS):
        cmd = HIWAVE+'-W -Prod=TBDML.ini -instance=tbdml '+os.path.abspath('%s/mc9s12xep100.abs'%(BDIR))+' -CMD="Go"'
        if('sim' in COMMAND_LINE_TARGETS):
            cmd = HIWAVE+'-W -Prod=Full_Chip_Simulation.ini -instance=sim '+os.path.abspath('%s/mc9s12xep100.abs'%(BDIR))
        print('cmd is:', cmd)
        with open('run9s12.bat','w') as fp:
            fp.write('@echo off\ncd %s/Project\n%s\n'%(cwd,cmd))
        if(0 != os.system('run9s12')):
            print('run of %s failed\n  %s\n'%(target, cmd))
        exit()
    CC   = cw + '/Prog/chc12.exe'
    AS   = cw + '/Prog/ahc12.exe'
    LINK = cw + '/Prog/linker.exe'
    S19  = cw + '/Prog/burner.exe'
    MAKE = '"{0}/Prog/piper.exe" "{0}/Prog/maker.exe"'.format(cw)
    with open('%s/makefile-%s.9s12'%(BDIR, target),'w') as fp:
        fp.write('CC = "%s"\n'%(CC))
        fp.write('AS = "%s"\n'%(AS))
        fp.write('LD = "%s"\n'%(LINK))
        fp.write('COMMON_FLAGS = -WErrFileOff -WOutFileOff -EnvOBJPATH=%s/%s\n'%(BDIR, target))
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
                if(IsNeedBuild(obj,target)):
                    fp.write(obj[:-2]+'.o ')
        fp.write('\n\nOBJS_LINK = ')
        for obj in objs:
            obj = str(obj)
            if(obj.endswith('.c') or obj.endswith('.C')):
                fp.write('%s/%s/%s.o '%(BDIR, target, os.path.basename(obj)[:-2]))
        fp.write('''\n
.asm.o:
    $(ASM) $*.asm $(COMMON_FLAGS) $(ASM_FLAGS)

.c.o:
    $(CC) $*.c $(INC) $(COMMON_FLAGS) $(C_FLAGS)

all:$(OBJS) {0}.abs
    {2} OPENFILE "{0}.s19" format=motorola busWidth=1 origin=0 len=0x80000000 destination=0 SRECORD=Sx SENDBYTE 1 "{0}.abs" CLOSE

{0}.abs :
    $(LD) {1} $(COMMON_FLAGS) $(LD_FLAGS) -Add($(OBJS_LINK)) -Add($(LIBS)) -M -O$*.abs'''.format(
        '%s/%s'%(BDIR,target), env['LINK_SCRIPTS'], S19)
    )
    cmd = '%s %s/makefile-%s.9s12'%(MAKE, BDIR, target)
    with open('%s/%s.bat'%(BDIR, target),'w') as fp:
        fp.write('@echo off\n%s\n'%(cmd))
    if(0 != os.system('%s/%s.bat'%(BDIR, target))):
        print('build of %s failed\n  %s\n'%(target, cmd))

asenv['Program'] = Program9S12

MODULES = ['DET',
           'MCU',
           'ECUM','SCHM',
           'CLIB_STDIO_PRINTF',
           'SHELL','RINGBUFFER','CLIB_STRTOK_R',
           'MPC9S12XEP100','COMMONXML'
           ]

print('INFO: for travil version of codewarrior, code size is limited to 32K,\n'
      'so CAN stack and VFS can\'t be enabled at the same time,\n'
      '  set CW32K_ONLY=CAN or VFS to choose between CAN stack and VFS\n'
      '  set CW32K_ONLY= to enable both CAN and VFS by default\n')
if((os.getenv('CW32K_ONLY') == None) or (os.getenv('CW32K_ONLY') == 'CAN')):
    MODULES += ['ASKAR']
    MODULES += ['CAN','CANIF','PDUR','CANTP','DCM','DCM_MINI']

if((os.getenv('CW32K_ONLY') == None) or (os.getenv('CW32K_ONLY') == 'VFS')):
    if(os.getenv('CW32K_ONLY') == 'VFS'):
        MODULES += ['TINYOS']
    MODULES += ['FATFS','VFS','DEV','SDCARD','CLIB_ASHEAP']

if('CLIB_ASHEAP' in MODULES):
    asenv.Append(CPPDEFINES=['configTOTAL_HEAP_SIZE=0x800'])

asenv.Append(CPPDEFINES=['USE_STD_ERROR'])

asenv['LINK_SCRIPTS'] = '%s/Project/prm/Project.prm'%(cwd)

# post special process for asboot release
if(asenv['RELEASE']=='asboot'):
    MODULES = ['DET',
               'CAN','CANTP','DCM',
               'FLASH','MCU',
               'ANYOS',
               'MINIBLT',
               'CANTP_MINI','DCM_MINI',
               'CLIB_STDIO_PRINTF',
               'SHELL','RINGBUFFER','CLIB_STRTOK_R',
               'MEM_CMD','JMP_CMD',
               'MPC9S12XEP100','BOOTLOADER'
           ]
    asenv['LINK_SCRIPTS'] = '%s/miniblt/prm/Project.prm'%(cwd)

    lds = '%s/com/as.application/board.bcm2835/script/linker-flsdrv.lds'%(ASROOT)
    asenv['flsdrv'] = {'CPPPATH':['%s/com/as.infrastructure/include'%(ASROOT),
                                  '%s/Project/Sources'%(cwd)],
                       'LINK_SCRIPTS':'%s/flsdrv/prm/Project.prm'%(cwd),
                       'objs':Glob('mcal/Flash.c')+Glob('miniblt/Sources/mc9s12xep100.c')}

ARCH='none'
arch='s12cpuv2'

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
    print('INFO: set DEBUG_ASCORE=YES to enable the debug of ascore')
    if(os.getenv('DEBUG_ASCORE')=='YES'):
        asenv.Append(CPPDEFINES=['DEBUG_ASCORE'])
        RunCommand('sed -i "224c VECTOR 0 _Startup" %s/Project/prm/Project.prm'%(cwd))
    else:
        RunCommand('sed -i "224c //VECTOR 0 _Startup" %s/Project/prm/Project.prm'%(cwd))

asenv.Append(CPPPATH=['%s/../../board.posix/common'%(cwd)])
asenv.Append(CPPDEFINES=['IS_ARCH16',
                         'ISR_ATTR=__near',
                         'OS_STK_SIZE_SCALER=1/4',
                         'ISR_ADDR(isr)=(uint16)isr',
                         '_G_va_list=va_list',
                         'OS_TICKS_PER_SECOND=100',
                         'DCM_DEFAULT_RXBUF_SIZE=512',
                         'DCM_DEFAULT_TXBUF_SIZE=512'])

if('MCU' in MODULES):
    objs += Glob('mcal/Mcu.c')
    objs += Glob('mcal/isr_os.xml')

if('CAN' in MODULES):
    objs += Glob('mcal/Can.c')
    objs += Glob('mcal/isr_can.xml')

if('CANIF' in MODULES):
    asenv.Append(CPPDEFINES=['CANIF_TASK_FIFO_MODE=STD_OFF'])

if('SHELL' in MODULES):
    if('MINIBLT' in MODULES):
        asenv.Append(CPPDEFINES=['USE_SHELL_WITHOUT_TASK'])
    asenv.Append(CPPDEFINES=['ENABLE_SHELL_ECHO_BACK'])
    asenv.Append(CPPDEFINES=['CMDLINE_MAX=64','FLASH_CMD_MAX_DATA=64'])

if('FLASH' in MODULES):
    asenv.Append(CPPDEFINES=['FLASH_DRIVER_STARTADDRESS=0x3900','FLASH_ERASE_SIZE=1024','FLASH_WRITE_SIZE=8'])

Return('objs')
