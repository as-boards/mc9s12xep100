from building import *

CWD = GetCurrentDir()

generate(Glob('config/*.json'))

cfgDcm = Glob('config/GEN/Dcm_Cfg.c')

ApplicationCanBL = query_application('CanBL')
ApplicationCanApp = query_application('CanApp')

objsCanBL = Glob('miniblt/Sources/*.c') + Glob('mcal/Can.c') + Glob('mcal/Mcu.c')

objsFlsDrv = Glob('mcal/Flash.c') + Glob('miniblt/Sources/mc9s12xep100.c')


@ register_application
class ApplicationMC9S12FlashDriver(Application):
    compiler = 'CWS12'

    def signit(self):
        sign = os.path.abspath('%s/../../GCC/Loader/Loader' % (BUILD_DIR))
        if IsPlatformWindows():
            sign += '.exe'
        if os.path.isfile(sign):
            target = '%s/MC9S12FlashDriver' % (BUILD_DIR)
            s19T = target + '.s19'
            cmd = '%s -f %s -s 1792 -S crc16' % (sign, s19T)
            self.AddPostAction(cmd)

    def config(self):
        self.CPPPATH = ['$INFRAS']
        self.Append(CPPPATH=['%s/miniblt/Sources' % (CWD), '%s/stdinc' % (CWD)])
        self.Append(LINKFLAGS=['-T', '%s/flsdrv/prm/Project.prm' % (CWD)])
        self.source = objsFlsDrv
        self.signit()


@ register_application
class ApplicationMC9S12CanBoot(ApplicationCanBL):
    compiler = 'CWS12'

    def platform_config(self):
        self.Append(CPPDEFINES=['BL_DISABLE_OTA', '__AS_BOOTLOADER__', 'DISABLE_CRC32',
                    'OS_TICKS_PER_SECOND=100', 'BL_USE_CRC_16', 'PDUR_DCM_CANTP_ZERO_COST',
                                'BL_USE_BUILTIN_FLS_READ', 'USE_STDIO_OUT', 'USE_STD_DEBUG',
                                'BL_FLSDRV_MEMORY_LOW=0x3900', 'BL_FLSDRV_MEMORY_HIGH=0x4000', 'FL_USE_WRITE_WINDOW_BUFFER',
                                'BL_APP_MEMORY_LOW=0x4000', 'BL_APP_MEMORY_HIGH=0xFAC000', 'BL_FINGER_PRINT_ADDRESS=0xFB8000',
                                'FLASH_DRIVER_STARTADDRESS=0x3900', 'FLASH_ERASE_SIZE=1024', 'FLASH_WRITE_SIZE=8'])
        self.Append(CPPPATH=['%s/miniblt/Sources' % (CWD), '%s/stdinc' % (CWD)])
        self.Append(LINKFLAGS=['-T', '%s/miniblt/prm/Project.prm' % (CWD)])

    def config(self):
        super().config()
        for libName in ['PduR']:
            if libName in self.LIBS:
                self.LIBS.remove(libName)
            macro = 'USE_%s' % (libName.upper())
            self.Remove(CPPDEFINES=[macro])
        self.RegisterConfig('Dcm', cfgDcm, force=True)
        self.LIBS.append('RingBuffer')
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
ApplicationLinApp = query_application('LinApp')


objsLinI2C = Glob('mcal/Lin_Slave_I2C_HW.c')
objsLinSPI = Glob('mcal/Lin_Slave_Spi.c')
objsLinBL = Glob('miniblt/Sources/*.c') + Glob('mcal/Mcu.c')
objsLinApp = Glob('Project/Sources/*.c') + Glob('mcal/Mcu.c') + Glob('config/vectors.c')

cfgLinSlaveI2C = Glob('config/Lin_Slave_I2C_Cfg.c')
cfgLinIf = Glob('config/LinIf_Cfg.c')
cfgLinTp = Glob('config/LinTp_Cfg.c')


@ register_application
class ApplicationMC9S12LinBoot(ApplicationLinBL):
    compiler = 'CWS12'

    def platform_config(self):
        self.Append(CPPDEFINES=['BL_DISABLE_OTA', '__AS_BOOTLOADER__', 'DISABLE_CRC32',
                    'OS_TICKS_PER_SECOND=100', 'BL_USE_CRC_16', 'PDUR_DCM_LINTP_ZERO_COST',
                                'BL_USE_BUILTIN_FLS_READ', 'USE_STDIO_OUT', 'USE_STD_DEBUG',
                                'BL_FLSDRV_MEMORY_LOW=0x3900', 'BL_FLSDRV_MEMORY_HIGH=0x4000', 'FL_USE_WRITE_WINDOW_BUFFER',
                                'BL_APP_MEMORY_LOW=0x4000', 'BL_APP_MEMORY_HIGH=0xFAC000', 'BL_FINGER_PRINT_ADDRESS=0xFB8000',
                                'FLASH_DRIVER_STARTADDRESS=0x3900', 'FLASH_ERASE_SIZE=1024', 'FLASH_WRITE_SIZE=8'])
        self.Append(CPPPATH=['%s/miniblt/Sources' % (CWD), '%s/stdinc' % (CWD)])
        self.Append(LINKFLAGS=['-T', '%s/miniblt/prm/Project.prm' % (CWD)])

    def config_lin_i2c(self):
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
        self.LIBS.append('RingBuffer')
        self.source += objsLinBL + objsLinI2C

    def config_lin_spi(self):
        super().config()
        self.RegisterConfig('Dcm', cfgDcm, force=True)
        self.RegisterConfig('LinTp', cfgLinTp, force=True)
        self.Append(CPPDEFINES=['LIN_USE_EXT_ID'])
        self.LIBS.append('RingBuffer')
        self.source += objsLinBL + objsLinSPI

    def config(self):
        self.config_lin_spi()


@ register_application
class ApplicationMC9S12LinApp(ApplicationLinApp):
    compiler = 'CWS12'

    def platform_config(self):
        self.Append(CPPDEFINES=['OS_TICKS_PER_SECOND=100', 'BL_FLSDRV_MEMORY_LOW=0x3900',
                    'PDUR_DCM_LINTP_ZERO_COST', 'USE_STDIO_OUT', 'USE_STD_DEBUG', ])
        self.Append(CPPPATH=['%s/Project/Sources' % (CWD), '%s/stdinc' % (CWD)])
        self.Append(LINKFLAGS=['-T', '%s/Project/prm/Project.prm' % (CWD)])

    def config_lin_i2c(self):
        super().config()
        for libName in ['LinIf', 'Dem', 'NvM', 'Fee', 'Fls', 'Ea', 'Eep']:
            if libName in self.LIBS:
                self.LIBS.remove(libName)
            macro = 'USE_%s' % (libName.upper())
            self.Remove(CPPDEFINES=[macro])
        self.LIBS.append('LinSlave:i2c')
        self.RegisterConfig('LinSlave:i2c', cfgLinSlaveI2C)
        self.Append(CPPDEFINES=['USE_LIN_SLAVE'])
        self.RegisterConfig('LinTp', cfgLinTp, force=True)
        self.LIBS.append('RingBuffer')
        self.source += objsLinApp + objsLinI2C

    def config_lin_spi(self):
        super().config()
        for libName in ['Dem', 'NvM', 'Fee', 'Fls', 'Ea', 'Eep']:
            if libName in self.LIBS:
                self.LIBS.remove(libName)
            macro = 'USE_%s' % (libName.upper())
            self.Remove(CPPDEFINES=[macro])
        self.RegisterConfig('LinTp', cfgLinTp, force=True)
        self.Append(CPPDEFINES=['LIN_USE_EXT_ID'])
        self.LIBS.append('RingBuffer')
        self.source += objsLinApp + objsLinSPI

    def config(self):
        self.config_lin_spi()


# build\nt\GCC\IsoTpSend\IsoTpSend.exe -t 0x0d -r 0x0f -d LIN.i2c/CH341_I2C -n 0xFFFF -D 20000 -b 100000 -v 1001

# build\nt\GCC\Loader\Loader.exe -t 0x51d0 -r 0x51d1 -d LIN.i2c/CH341_I2C -n 0xFFFF -D 20000 -b 100000 -S crc16 -a build/nt/GCC/MC9S12LinApp/MC9S12LinApp.s19 -f build\nt\GCC\MC9S12FlashDriver\MC9S12FlashDriver.s19.sign

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
