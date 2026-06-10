// use TMC4671 API
#include "hal/ic/TMC4671.h"

// Motor type &  PWM configuration
tmc4671_writeInt(0, TMC4671_MOTOR_TYPE_N_POLE_PAIRS, 0x0003000B);
tmc4671_writeInt(0, TMC4671_PWM_POLARITIES, 0x00000000);
tmc4671_writeInt(0, TMC4671_PWM_MAXCNT, 0x00000F9F);
tmc4671_writeInt(0, TMC4671_PWM_BBM_H_BBM_L, 0x00002828);
tmc4671_writeInt(0, TMC4671_PWM_SV_CHOP, 0x00000007);

// ADC configuration
tmc4671_writeInt(0, TMC4671_ADC_I_SELECT, 0x18000100);
tmc4671_writeInt(0, TMC4671_dsADC_MCFG_B_MCFG_A, 0x00100010);
tmc4671_writeInt(0, TMC4671_dsADC_MCLK_A, 0x20000000);
tmc4671_writeInt(0, TMC4671_dsADC_MCLK_B, 0x20000000);
tmc4671_writeInt(0, TMC4671_dsADC_MDEC_B_MDEC_A, 0x014E014E);
tmc4671_writeInt(0, TMC4671_ADC_I0_SCALE_OFFSET, 0x010081D5);
tmc4671_writeInt(0, TMC4671_ADC_I1_SCALE_OFFSET, 0x01008133);

// ABN encoder settings
tmc4671_writeInt(0, TMC4671_ABN_DECODER_MODE, 0x0000100F);
tmc4671_writeInt(0, TMC4671_ABN_DECODER_PPR, 0x00001000);
tmc4671_writeInt(0, TMC4671_ABN_DECODER_COUNT, 0x00000025);
tmc4671_writeInt(0, TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, 0x00000000);

// Limits
tmc4671_writeInt(0, TMC4671_PID_TORQUE_FLUX_LIMITS, 0x000003E8);

// PI settings
tmc4671_writeInt(0, TMC4671_PID_TORQUE_P_TORQUE_I, 0x01000100);
tmc4671_writeInt(0, TMC4671_PID_FLUX_P_FLUX_I, 0x01000100);

// ===== ABN encoder test drive =====

// Init encoder (mode 0)
tmc4671_writeInt(0, TMC4671_MODE_RAMP_MODE_MOTION, 0x00000008); // 1. Switches to Open Loop Mode
tmc4671_writeInt(0, TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, 0x00000000);
tmc4671_writeInt(0, TMC4671_PHI_E_SELECTION, 0x00000001); // 2. Uses the internal Phi_E_Ext angle allocator
tmc4671_writeInt(0, TMC4671_PHI_E_EXT, 0x00000000); // 3. Sets the electrical target angle to exactly 0°
tmc4671_writeInt(0, TMC4671_UQ_UD_EXT, 0x00000FA0); // 4. Applies a strong voltage (Ud) to force lock the rotor
wait(1000); // 5. Waits 1 second for the motor to physically snap and settle
tmc4671_writeInt(0, TMC4671_ABN_DECODER_COUNT, 0x00000000); // 6. Resets the encoder counter to 0 at this physical position

// Feedback selection
tmc4671_writeInt(0, TMC4671_PHI_E_SELECTION, 0x00000003); // Switches FOC commutation source to the ABN Encoder!
tmc4671_writeInt(0, TMC4671_VELOCITY_SELECTION, 0x00000009);

// Switch to torque mode
tmc4671_writeInt(0, TMC4671_MODE_RAMP_MODE_MOTION, 0x00000001); // Switches the chip into Torque Mode

// Rotate right
tmc4671_writeInt(0, TMC4671_PID_TORQUE_FLUX_TARGET, 0x03E80000);
wait(3000);

// Rotate left
tmc4671_writeInt(0, TMC4671_PID_TORQUE_FLUX_TARGET, 0xFC180000);
wait(3000);

// Stop
tmc4671_writeInt(0, TMC4671_PID_TORQUE_FLUX_TARGET, 0x00000000);
