from building import *

CWD = GetCurrentDir()

ApplicationCanBL = query_application('CanBL')
ApplicationCanApp = query_application('CanApp')

objsBL = Glob('miniblt/Sources/*.c') + Glob('mcal/Can.c') + Glob('mcal/Mcu.c')


@ register_application
class ApplicationMC9S12Boot(ApplicationCanBL):
    compiler = 'CWS12'

    def platform_config(self):
        self.Append(CPPDEFINES=['BL_DISABLE_OTA', '__AS_BOOTLOADER__', 'DISABLE_CRC32',
                    'OS_TICKS_PER_SECOND=100', 'BL_USE_CRC_16', 'PDUR_DCM_CANTP_ZERO_COST',
                                'FLASH_DRIVER_STARTADDRESS=0x3900', 'FLASH_ERASE_SIZE=1024', 'FLASH_WRITE_SIZE=8'])
        self.Append(CPPPATH=['%s/miniblt/Sources' % (CWD)])
        self.Append(LINKFLAGS=['-T', '%s/miniblt/prm/Project.prm' % (CWD)])

    def config(self):
        super().config()
        for libName in ['PduR']:
            if libName in self.LIBS:
                self.LIBS.remove(libName)
            macro = 'USE_%s' % (libName.upper())
            self.Remove(CPPDEFINES=[macro])
        self.source += objsBL


@register_application
class ApplicationMC9S12BootRun(Application):
    compiler = 'CWS12'

    def config(self):
        pass

    def build(self):
        BDIR = BUILD_DIR[:-3]
        HIWAVE = '"%s/Prog/hiwave.exe" ' % (GetCWS12DIR())
        cmd = HIWAVE+'-W -Prod=TBDML.ini -instance=tbdml ' + \
            os.path.abspath('%s/MC9S12Boot.abs' % (BDIR))+' -CMD="Go"'
        if ('sim' in COMMAND_LINE_TARGETS):
            cmd = HIWAVE+'-W -Prod=Full_Chip_Simulation.ini -instance=sim ' + \
                os.path.abspath('%s/MC9S12Boot.abs' % (BDIR))
        with open('%s/run9s12.bat' % (BDIR), 'w') as fp:
            fp.write('@echo off\ncd %s/miniblt\n%s\n' % (CWD, cmd))
        RunCommand('%s/run9s12.bat' % (BDIR))
