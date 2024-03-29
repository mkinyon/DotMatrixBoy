#pragma once

namespace Core
{
	inline const unsigned int RAM_SIZE = 32768;
	constexpr float CYCLES_PER_MS = 4194.304f;

	typedef enum
	{
		/* Input */
		HW_FF00_P1JOYP_JOYPAD = 0xFF00,


		/* Misc */
		HW_FF01_SB_SERIAL_TRANSFER_DATA = 0xFF01,
		HW_FF02_SC_SERIAL_TRANSFER_CONTROL = 0xFF02,

		HW_FF03_DIV_DIVIDER_REGISTER_LOW = 0xFF03,
		HW_FF04_DIV_DIVIDER_REGISTER = 0xFF04, // 16 bit
		HW_FF05_TIMA_TIMER_COUNTER = 0xFF05,
		HW_FF06_TMA_TIMER_MODULO = 0xFF06,
		HW_FF07_TAC_TIMER_CONTROL = 0xFF07,

		HW_FF0F_IF_INTERRUPT_FLAG = 0xFF0F,


		/* Audio */
		HW_FF10_NR10_SOUND_CH1_SWEEP = 0xFF10,
		HW_FF11_NR11_SOUND_CH1_LEN_TIMER = 0xFF11,
		HW_FF12_NR12_SOUND_CH1_VOL_ENVELOPE = 0xFF12,
		HW_FF13_NR13_SOUND_CH1_PERIOD_LOW = 0xFF13,
		HW_FF14_NR14_SOUND_CH1_PERIOD_HIGH = 0xFF14,
		HW_FF16_NR21_SOUND_CH2_LEN_TIMER = 0xFF16,
		HW_FF17_NR22_SOUND_CH2_VOL_ENVELOPE = 0xFF17,
		HW_FF18_NR23_SOUND_CH2_PERIOD_LOW = 0xFF18,
		HW_FF19_NR24_SOUND_CH2_PERIOD_HIGH = 0xFF19,
		HW_FF1A_NR30_SOUND_CH3_DAC_ENABLE = 0xFF1A,
		HW_FF1B_NR31_SOUND_CH3_LEN_TIMER = 0xFF1B,
		HW_FF1C_NR32_SOUND_CH3_OUTPUT_LEVEL = 0xFF1C,
		HW_FF1D_NR33_SOUND_CH3_PERIOD_LOW = 0xFF1D,
		HW_FF1E_NR34_SOUND_CH3_PERIOD_HIGH = 0xFF1E,
		HW_FF20_NR41_SOUND_CH4_LEN_TIMER = 0xFF20,
		HW_FF21_NR42_SOUND_CH4_VOL_ENVELOPE = 0xFF21,
		HW_FF22_NR43_SOUND_CH4_FREQ_RANDOM = 0xFF22,
		HW_FF23_NR44_SOUND_CH4_CONTROL = 0xFF23,
		HW_FF24_NR50_MASTER_VOLUME = 0xFF24,
		HW_FF25_NR51_SOUND_PANNING = 0xFF25,
		HW_FF26_NR52_SOUND_TOGGLE = 0xFF26,

		HW_FF30_WAVRAM_WAVEFORM_STORAGE_0 = 0xFF30,
		HW_FF31_WAVRAM_WAVEFORM_STORAGE_1 = 0xFF31,
		HW_FF32_WAVRAM_WAVEFORM_STORAGE_2 = 0xFF32,
		HW_FF33_WAVRAM_WAVEFORM_STORAGE_3 = 0xFF33,
		HW_FF34_WAVRAM_WAVEFORM_STORAGE_4 = 0xFF34,
		HW_FF35_WAVRAM_WAVEFORM_STORAGE_5 = 0xFF35,
		HW_FF36_WAVRAM_WAVEFORM_STORAGE_6 = 0xFF36,
		HW_FF37_WAVRAM_WAVEFORM_STORAGE_7 = 0xFF37,
		HW_FF38_WAVRAM_WAVEFORM_STORAGE_8 = 0xFF38,
		HW_FF39_WAVRAM_WAVEFORM_STORAGE_9 = 0xFF39,
		HW_FF3A_WAVRAM_WAVEFORM_STORAGE_A = 0xFF3A,
		HW_FF3B_WAVRAM_WAVEFORM_STORAGE_B = 0xFF3B,
		HW_FF3C_WAVRAM_WAVEFORM_STORAGE_C = 0xFF3C,
		HW_FF3D_WAVRAM_WAVEFORM_STORAGE_D = 0xFF3D,
		HW_FF3E_WAVRAM_WAVEFORM_STORAGE_E = 0xFF3E,
		HW_FF3F_WAVRAM_WAVEFORM_STORAGE_F = 0xFF3F,


		/* Video */
		HW_FF40_LCDC_LCD_CONTROL = 0xFF40,
		HW_FF41_STAT_LCD_STATUS = 0xFF41,

		HW_FF42_SCY_VIEWPORT_Y_POS = 0xFF42,
		HW_FF43_SCX_VIEWPORT_X_POS = 0xFF43,

		HW_FF44_LY_LCD_Y_COORD = 0xFF44,
		HW_FF45_LYC_LY_COMPARE = 0xFF45,

		HW_FF46_DMA_OAM_DMA_SOURCE_ADDRESS = 0xFF46,
		HW_FF47_BGP_BG_PALETTE_DATA = 0xFF47,
		HW_FF48_OBP0_OBJ_PALETTE_0_DATA = 0xFF48,
		HW_FF49_OBP1_OBJ_PALETTE_1_DATA = 0xFF49,

		HW_FF4A_WY_WINDOW_Y_POS = 0xFF4A,
		HW_FF4B_WX_WINDOW_X_POS = 0xFF4B,


		/* Interrupt Toggle */
		HW_FFFF_INTERRUPT_ENABLE = 0xFFFF

	} HardwareAddresses;

	/********************************************************************************************
		MMU Specific
	*********************************************************************************************/

	inline const uint16_t CART_ADDR_RANGE_START = 0x0000;
	inline const uint16_t CART_ADDR_RANGE_END = 0x7FFF;

	inline const uint16_t CARTBANK_ADDR_RANGE_START = 0x4000;
	inline const uint16_t CARTBANK_ADDR_RANGE_END = 0x7FFF;

	inline const uint16_t CARTRAM_ADDR_RANGE_START = 0xA000;
	inline const uint16_t CARTRAM_ADDR_RANGE_END = 0xBFFF;

	// MBC 1
	inline const uint16_t MBC1_RAMG_START = 0x0000;
	inline const uint16_t MBC1_RAMG_END = 0x1FFF;
	inline const uint16_t MBC1_BANKREG1_START = 0x2000;
	inline const uint16_t MBC1_BANKREG1_END = 0x3FFF;
	inline const uint16_t MBC1_BANKREG2_START = 0x4000;
	inline const uint16_t MBC1_BANKREG2_END = 0x5FFF;
	inline const uint16_t MBC1_MODE_START = 0x6000;
	inline const uint16_t MBC1_MODE_END = 0x7FFF;

	inline const uint16_t ROM_BANK_SIZE = 0x4000;
	inline const uint16_t RAM_BANK_SIZE = 0x2000;

	inline const uint16_t APU_RANGE_START = 0xFF10;
	inline const uint16_t APU_RANGE_END = 0xFF3F;


	/********************************************************************************************
		CPU Specific
	*********************************************************************************************/
	inline const uint8_t DEST_ADDRESS_VBLANK = 0x0040;
	inline const uint8_t DEST_ADDRESS_LCD_STAT = 0x0048;
	inline const uint8_t DEST_ADDRESS_TIMER = 0x0050;
	inline const uint8_t DEST_ADDRESS_SERIAL = 0x0058;
	inline const uint8_t DEST_ADDRESS_JOYPAD = 0x0060;

	typedef enum
	{
		FLAG_CARRY = 0x10, // C 0001 0000
		FLAG_HALF_CARRY = 0x20, // H 0010 0000
		FLAG_SUBTRACT = 0x40, // N 0100 0000
		FLAG_ZERO = 0x80 // Z 1000 0000
	} Cpu_Flags;

	typedef enum
	{
		IE_VBLANK = 0x01,
		IE_LCD = (0x01 << 1),
		IE_TIMER = (0x01 << 2),
		IE_SERIAL = (0x01 << 3),
		IE_JOYPAD = (0x01 << 4),
	} IE_Flags;

	typedef enum
	{
		IF_VBLANK = 0x01,
		IF_LCD = (0x01 << 1),
		IF_TIMER = (0x01 << 2),
		IF_SERIAL = (0x01 << 3),
		IF_JOYPAD = (0x01 << 4),
	} IF_Flags;

	typedef enum
	{
		TAC_CLOCK_SELECT_LBIT = 0x01,
		TAC_CLOCK_SELECT_HBIT = (0x01 << 1),
		TAC_ENABLE = (0x01 << 2)
	} TimerControl_Flags;


	/********************************************************************************************
		PPU Specific
	*********************************************************************************************/

	inline const int LCD_WIDTH = 160;
	inline const int LCD_HEIGHT = 144;
	inline const int TILEMAP_WIDTH = 32;
	inline const int TILEMAP_HEIGHT = 32;
	inline const int TILEMAP_WIDTH_PIXELS = 256;
	inline const int TILEMAP_HEIGHT_PIXELS = 256;
	inline const int TILE_WIDTH = 8;
	inline const int TILE_HEIGHT = 8;

	inline const int OAM_CYCLES = 80;
	inline const int VBLANK_CYCLES = 456;
	inline const int HBLANK_CYCLES = 204;
	inline const int DRAWING_CYCLES = 173;

	inline const uint16_t BG_MAP_0 = 0x9800;
	inline const uint16_t BG_MAP_1 = 0x9C00;

	typedef enum
	{
		MODE_0_HBLANK = 0,  // Horizontal Blank
		MODE_1_VBLANK = 1,  // Vertical Blank
		MODE_2_OAMSCAN = 2, // OAM Scan
		MODE_3_DRAWING = 3  // Drawing Pixels
	} LCD_Mode;

	typedef enum
	{
		LCDC_BG_WINDOW_ENABLE_PRIORITY = 0x01,
		LCDC_OBJ_ENABLE = (0x01 << 1),
		LCDC_OBJ_SIZE = (0x01 << 2),
		LCDC_BG_TILE_MAP = (0x01 << 3),
		LCDC_BG_AND_WINDOW_TILES = (0x01 << 4),
		LCDC_WINDOW_ENABLE = (0x01 << 5),
		LCDC_WINDOW_TILE_MAP = (0x01 << 6),
		LCDC_LCD_PPU_ENABLE = (0x01 << 7)
	} LCDC_Flags;

	typedef enum
	{
		STAT_PPU_MODE_LBIT = 0x01,
		STAT_PPU_MODE_HBIT = (0x01 << 1),
		STAT_LYC_EQUAL_LY = (0x01 << 2),
		STAT_MODE_0_INT_SELECT = (0x01 << 3),
		STAT_MODE_1_INT_SELECT = (0x01 << 4),
		STAT_MODE_2_INT_SELECT = (0x01 << 5),
		STATE_LYC_INT_SELECT = (0x01 << 6)
	} STAT_FLags;

	typedef enum
	{
		OAM_PALETTE = (0x01 << 4),
		OAM_FLIP_X = (0x01 << 5),
		OAM_FLIP_Y = (0x01 << 6),
		OAM_PRIORITY = (0x01 << 7)
	} OAM_Flags;


	/********************************************************************************************
		APU Specific
	*********************************************************************************************/

	inline const uint8_t APU_DEFAULT_LENGTH = 64;
	inline const unsigned int AUDIO_SAMPLE_SIZE = 4096;
	inline const uint16_t AUDIO_WAVE_LENGTH = 256;

	typedef enum
	{
		MUTE = 0,
		LEVEL_100 = 1,
		LEVEL_50 = 2,
		LEVEL_25 = 3
	} AUDIO_LEVEL;

	// NR10: Channel 1 sweep
	typedef enum
	{
		NR10_INV_STEP_LBIT = 0x01,
		NR10_INV_STEP_MBIT = (0x01 << 1),
		NR10_INV_STEP_HBIT = (0x01 << 2),
		NR10_DIRECTION =     (0x01 << 3),
		NR10_PACE_LBIT =	 (0x01 << 4),
		NR10_PACE_MBIT =	 (0x01 << 5),
		NR10_PACE_HBIT =	 (0x01 << 6)
	} NR10_Flags;

	// NR11: Channel 1 length timer & duty cycle
	typedef enum
	{
		NR11_INIT_LEN_TIMER_BIT0 = 0x01,
		NR11_INIT_LEN_TIMER_BIT1 = (0x01 << 1),
		NR11_INIT_LEN_TIMER_BIT2 = (0x01 << 2),
		NR11_INIT_LEN_TIMER_BIT3 = (0x01 << 3),
		NR11_INIT_LEN_TIMER_BIT4 = (0x01 << 4),
		NR11_INIT_LEN_TIMER_BIT5 = (0x01 << 5),
		NR11_DUTY_CYCLE_LBIT =     (0x01 << 6),
		NR11_DUTY_CYCLE_HBIT =     (0x01 << 7)
	} NR11_Flags;

	// NR12: Channel 1 volume & envelope
	typedef enum
	{
		NR12_SWEEP_PACE_LBIT = 0x01,
		NR12_SWEEP_PACE_MBIT = (0x01 << 1),
		NR12_SWEEP_PACE_HBIT = (0x01 << 2),
		NR12_ENV_DIR_LBIT = (0x01 << 3),
		NR12_ENV_DIR_HBIT = (0x01 << 4),
		NR12_INIT_VOL_LBIT = (0x01 << 5),
		NR12_INIT_VOL_MBIT = (0x01 << 6),
		NR12_INIT_VOL_HBIT = (0x01 << 7)
	} NR12_Flags;

	// NR14: Channel 1 period high & control
	typedef enum
	{
		NR14_PERIOD_BIT0 = 0x01,
		NR14_PERIOD_BIT1 = (0x01 << 1),
		NR14_PERIOD_BIT2 = (0x01 << 2),
		NR14_PERIOD_BIT3 = (0x01 << 3),
		NR14_PERIOD_BIT4 = (0x01 << 4),
		NR14_PERIOD_BIT5 = (0x01 << 5),
		NR14_LEN_ENABLE =  (0x01 << 6),
		NR14_TRIGGER =     (0x01 << 7)
	} NR14_Flags;

	// NR30: Channel 3 DAC enable
	typedef enum
	{
		NR30_DAC_ON_OFF = (0x01 << 7)
	} NR30_Flags;

	// NR32: Channel 3 output level
	typedef enum
	{
		NR32_OUTPUT_LEVEL_LBIT = (0x01 << 5),
		NR32_OUTPUT_LEVEL_HBIT = (0x01 << 6),
	} NR32_Flags;

	// NR34: Channel 3 period high & control
	typedef enum
	{
		NR34_PERIOD_BIT0 = 0x01,
		NR34_PERIOD_BIT1 = (0x01 << 1),
		NR34_PERIOD_BIT2 = (0x01 << 2),
		NR34_PERIOD_BIT3 = (0x01 << 3),
		NR34_PERIOD_BIT4 = (0x01 << 4),
		NR34_PERIOD_BIT5 = (0x01 << 5),
		NR34_LEN_ENABLE = (0x01 << 6),
		NR34_TRIGGER = (0x01 << 7)
	} NR34_Flags;

	// NR41: Channel 4 length timer
	typedef enum
	{
		NR41_INIT_LEN_TIMER_BIT0 = 0x01,
		NR41_INIT_LEN_TIMER_BIT1 = (0x01 << 1),
		NR41_INIT_LEN_TIMER_BIT2 = (0x01 << 2),
		NR41_INIT_LEN_TIMER_BIT3 = (0x01 << 3),
		NR41_INIT_LEN_TIMER_BIT4 = (0x01 << 4),
		NR41_INIT_LEN_TIMER_BIT5 = (0x01 << 5)
	} NR41_Flags;

	// NR43: Channel 4 frequency & randomness
	typedef enum
	{
		NR43_CLK_DIV_LBIT = 0x01,
		NR43_CLK_DIV_MBIT = (0x01 << 1),
		NR43_CLK_DIV_HBIT = (0x01 << 2),
		NR43_LFSR_WIDTH = (0x01 << 3),
		NR43_CLOCK_SHFT_BIT0 = (0x01 << 4),
		NR43_CLOCK_SHFT_BIT1 = (0x01 << 5),
		NR43_CLOCK_SHFT_BIT2 = (0x01 << 6),
		NR43_CLOCK_SHFT_BIT3 = (0x01 << 7)
	} NR43_Flags;

	// NR44: Channel 4 control
	typedef enum
	{
		NR44_LEN_ENABLE = (0x01 << 6),
		NR44_TRIGGER = (0x01 << 7)
	} NR44_Flags;

	// NR50: Master volume & VIN panning
	typedef enum
	{
		NR50_R_VOL_LBIT = 0x01,
		NR50_R_VOL_MBIT = (0x01 << 1),
		NR50_R_VOL_HBIT = (0x01 << 2),
		NR50_VIN_R =      (0x01 << 3),
		NR50_L_VOL_LBIT = (0x01 << 4),
		NR50_L_VOL_MBIT = (0x01 << 5),
		NR50_L_VOL_HBIT = (0x01 << 6),
		NR50_VIN_L =      (0x01 << 7)
	} NR50_Flags;

	// NR51: Sound panning
	typedef enum
	{
		NR51_CH1_R = 0x01,
		NR51_CH2_R = (0x01 << 1),
		NR51_CH3_R = (0x01 << 2),
		NR51_CH4_R = (0x01 << 3),
		NR51_CH1_L = (0x01 << 4),
		NR51_CH2_L = (0x01 << 5),
		NR51_CH3_L = (0x01 << 6),
		NR51_CH4_L = (0x01 << 7)
	} NR51_Flags;

	// NR52: Audio master control
	typedef enum
	{
		NR52_CH1_ON = 0x01,
		NR52_CH2_ON = (0x01 << 1),
		NR52_CH3_ON = (0x01 << 2),
		NR52_CH4_ON = (0x01 << 3),
		NR52_AUDIO_ON = (0x01 << 7)
	} NR52_Flags;


	/********************************************************************************************
		Joypad
	*********************************************************************************************/

	/* 	
		The 8 buttons (dpad + face buttons) are arranged in a 2x4 matrix.
	
			+-----------------+-------+--------+-------+-------+
			|  Mask           | Bit 3 | Bit 2  | Bit 1 | Bit 0 |
			+-----------------+-------+--------+-------+-------+
			| Buttons (bit 5) | Start | Select | B     | A     |
			+-----------------+-------+--------+-------+-------+
			| DPAD    (bit 4) | Down  | Up     | Left  | Right |
			+-----------------+-------+--------+-------+-------+

		- If the DPAD (bit 4) is zero, then the directional buttons can be read.
		- If the Buttons (bit5) is zero, then the face buttons can be read.
		- Buttons (Bits 3 - 0) are considered pressed is set to 0. 1 means the buttons are released.
	*/

	typedef enum
	{
		JOYP_A_RIGHT = 0x01,
		JOYP_B_LEFT = (0x01 << 1),
		JOYP_SELECT_UP = (0x01 << 2),
		JOYP_START_DOWN = (0x01 << 3),
		JOYP_SELECT_DPAD = (0x01 << 4),
		JOYP_SELECT_BUTTONS = (0x01 << 5),
	} JOYPAD_Flags;

	// These values are arranged in specific order so they line up with the matrix above..
	//  i.e.. Right is paired with the A button.
	typedef enum
	{
		RIGHT = 0x01,
		LEFT = (0x01 << 1),
		UP = (0x01 << 2),
		DOWN = (0x01 << 3),
	} Joypad_DPAD;

	typedef enum
	{
		A = 0x01,
		B = (0x01 << 1),
		SELECT = (0x01 << 2),
		START = (0x01 << 3),
	} Joypad_Button;
}