isr_pc = {39: 'Can_0_TxIsr_Entry', 38: 'Can_0_RxIsr_Entry',
          37: 'Can_0_ErrIsr_Entry', 36: 'Can_0_WakeIsr_Entry',
          31: 'Lin_Slave_I2C_ISR', 7: 'Isr_SystemTick', 4: 'Isr_SoftwareInterrupt'}


for k, v in isr_pc.items():
    print('extern interrupt void %s(void);' % (v))

for i in range(120):
  I = 119 - i
  api = isr_pc[I] if I in isr_pc else 'isr_default'
  print('  /* %s */ %s,'%(I, api))
