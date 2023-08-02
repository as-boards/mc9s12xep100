from building import *

CWD = GetCurrentDir()

generate(Glob('config/*.json'))

cfgDcm = Glob('config/GEN/Dcm_Cfg.c')

ApplicationCanBL = query_application('CanBL')
ApplicationCanApp = query_application('CanApp')

objsCanBL = Glob('miniblt/Sources/*.c') + Glob('mcal/Can.c') + Glob('mcal/Mcu.c')

@ register_application
class ApplicationMC9S12CanBoot(ApplicationCanBL):
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
        self.RegisterConfig('Dcm', cfgDcm, force=True)
        self.source += objsCanBL


@register_application
class ApplicationMC9S12CanBootRun(Application):
    compiler = 'CWS12'

    def config(self):
        pass

    def build(self):
        BDIR = BUILD_DIR[:-3]
        HIWAVE = '"%s/Prog/hiwave.exe" ' % (GetCWS12DIR())
        cmd = HIWAVE+'-W -Prod=TBDML.ini -instance=tbdml ' + \
            os.path.abspath('%s/MC9S12CanBoot.abs' % (BDIR))+' -CMD="Go"'
        if ('sim' in COMMAND_LINE_TARGETS):
            cmd = HIWAVE+'-W -Prod=Full_Chip_Simulation.ini -instance=sim ' + \
                os.path.abspath('%s/MC9S12CanBoot.abs' % (BDIR))
        with open('%s/run9s12.bat' % (BDIR), 'w') as fp:
            fp.write('@echo off\ncd %s/miniblt\n%s\n' % (CWD, cmd))
        RunCommand('%s/run9s12.bat' % (BDIR))


ApplicationLinBL = query_application('LinBL')

objsLinBL = Glob('miniblt/Sources/*.c') + Glob('mcal/Mcu.c') + Glob('mcal/Lin_Slave_I2C_HW.c')

cfgLinSlaveI2C = Glob('config/Lin_Slave_I2C_Cfg.c')
cfgLinTp = Glob('config/LinTp_Cfg.c')

@ register_application
class ApplicationMC9S12LinBoot(ApplicationLinBL):
    compiler = 'CWS12'

    def platform_config(self):
        self.Append(CPPDEFINES=['BL_DISABLE_OTA', '__AS_BOOTLOADER__', 'DISABLE_CRC32',
                    'OS_TICKS_PER_SECOND=100', 'BL_USE_CRC_16', 'PDUR_DCM_LINTP_ZERO_COST',
                                'FLASH_DRIVER_STARTADDRESS=0x3900', 'FLASH_ERASE_SIZE=1024', 'FLASH_WRITE_SIZE=8'])
        self.Append(CPPPATH=['%s/miniblt/Sources' % (CWD)])
        self.Append(LINKFLAGS=['-T', '%s/miniblt/prm/Project.prm' % (CWD)])

    def config(self):
        super().config()
        for libName in ['LinIf']:
            if libName in self.LIBS:
                self.LIBS.remove(libName)
            macro = 'USE_%s' % (libName.upper())
            self.Remove(CPPDEFINES=[macro])
        self.LIBS.append('LinSlave:i2c')
        self.RegisterConfig('LinSlave:i2c', cfgLinSlaveI2C)
        self.Append(CPPDEFINES=['USE_LIN_SLAVE'])
        self.RegisterConfig('Dcm', cfgDcm, force=True)
        self.RegisterConfig('LinTp', cfgLinTp, force=True)
        self.source += objsLinBL


# build\nt\GCC\IsoTpSend\IsoTpSend.exe -v 1001 -t 0x0d -r 0x0f -b 20000 -d LIN.i2c/CH341_I2C -a 0xFFFF -s 20000

@register_application
class ApplicationMC9S12LinBootRun(Application):
    compiler = 'CWS12'

    def config(self):
        pass

    def build(self):
        BDIR = BUILD_DIR[:-3]
        HIWAVE = '"%s/Prog/hiwave.exe" ' % (GetCWS12DIR())
        cmd = HIWAVE+'-W -Prod=TBDML.ini -instance=tbdml ' + \
            os.path.abspath('%s/MC9S12LinBoot.abs' % (BDIR))+' -CMD="Go"'
        if ('sim' in COMMAND_LINE_TARGETS):
            cmd = HIWAVE+'-W -Prod=Full_Chip_Simulation.ini -instance=sim ' + \
                os.path.abspath('%s/MC9S12LinBoot.abs' % (BDIR))
        with open('%s/run9s12.bat' % (BDIR), 'w') as fp:
            fp.write('@echo off\ncd %s/miniblt\n%s\n' % (CWD, cmd))
        RunCommand('%s/run9s12.bat' % (BDIR))
