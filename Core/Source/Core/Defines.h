#pragma once

namespace Core
{
	constexpr float CYCLES_PER_MS = 4194.304f;

	typedef enum
	{
		/* Input */
		HW_P1JOYP_JOYPAD = 0xFF00,


		/* Misc */
		HW_SB_SERIAL_TRANSFER_DATA = 0xFF01,
		HW_SC_SERIAL_TRANSFER_CONTROL = 0xFF02,

		HW_DIV_DIVIDER_REGISTER_LOW = 0xFF03,
		HW_DIV_DIVIDER_REGISTER = 0xFF04, // 16 bit
		HW_TIMA_TIMER_COUNTER = 0xFF05,
		HW_TMA_TIMER_MODULO = 0xFF06,
		HW_TAC_TIMER_CONTROL = 0xFF07,

		HW_IF_INTERRUPT_FLAG = 0xFF0F,


		/* Audio */
		HW_NR10_SOUND_CHANNEL_1_SWEEP = 0xFF10,
		HW_NR11_SOUND_CHANNEL_1_LEN_TIMER = 0xFF11,
		HW_NR12_SOUND_CHANNEL_1_VOL_ENVELOPE = 0xFF12,
		HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW = 0xFF13,
		HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH = 0xFF14,
		HW_NR21_SOUND_CHANNEL_2_LEN_TIMER = 0xFF16,
		HW_NR22_SOUND_CHANNEL_2_VOL_ENVELOPE = 0xFF17,
		HW_NR23_SOUND_CHANNEL_2_PERIOD_LOW = 0xFF18,
		HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH = 0xFF19,
		HW_NR30_SOUND_CHANNEL_3_DAC_ENABLE = 0xFF1A,
		HW_NR31_SOUND_CHANNEL_3_LEN_TIMER = 0xFF1B,
		HW_NR32_SOUND_CHANNEL_3_OUTPUT_LEVEL = 0xFF1C,
		HW_NR33_SOUND_CHANNEL_3_PERIOD_LOW = 0xFF1D,
		HW_NR34_SOUND_CHANNEL_3_PERIOD_HIGH = 0xFF1E,
		HW_NR41_SOUND_CHANNEL_4_LEN_TIMER = 0xFF20,
		HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE = 0xFF21,
		HW_NR43_SOUND_CHANNEL_4_FREQ_RANDOM = 0xFF22,
		HW_NR44_SOUND_CHANNEL_4_CONTROL = 0xFF23,
		HW_NR50_MASTER_VOLUME = 0xFF24,
		HW_NR51_SOUND_PANNING = 0xFF25,
		HW_NR52_SOUND_TOGGLE = 0xFF26,
		HW_WAVRAM_WAVEFORM_STORAGE = 0xFF30, // $FF30-FF3F


		/* Video */
		HW_LCDC_LCD_CONTROL = 0xFF40,
		HW_STAT_LCD_STATUS = 0xFF41,

		HW_SCY_VIEWPORT_Y_POS = 0xFF42,
		HW_SCX_VIEWPORT_X_POS = 0xFF43,

		HW_LY_LCD_Y_COORD = 0xFF44,
		HW_LYC_LY_COMPARE = 0xFF45,

		HW_DMA_OAM_DMA_SOURCE_ADDRESS = 0xFF46,
		HW_BGP_BG_PALETTE_DATA = 0xFF47,
		HW_OBP0_OBJ_PALETTE_0_DATA = 0xFF48,
		HW_OBP1_OBJ_PALETTE_1_DATA = 0xFF49,

		HW_WY_WINDOW_Y_POS = 0xFF4A,
		HW_WX_WINDOW_X_POS = 0xFF4B,


		/* Interrupt Toggle */
		HW_INTERRUPT_ENABLE = 0xFFFF

	} HardwareAddresses;

	/********************************************************************************************
		CPU Specific
	*********************************************************************************************/
	#define DEST_ADDRESS_VBLANK 0x0040 
	#define DEST_ADDRESS_LCD_STAT 0x0048
	#define DEST_ADDRESS_TIMER 0x0050
	#define DEST_ADDRESS_SERIAL 0x0058
	#define DEST_ADDRESS_JOYPAD 0x0060

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
	} InterruptEnable_Flags;

	typedef enum
	{
		IF_VBLANK = 0x01,
		IF_LCD = (0x01 << 1),
		IF_TIMER = (0x01 << 2),
		IF_SERIAL = (0x01 << 3),
		IF_JOYPAD = (0x01 << 4),
	} Interrupt_Flags;

	typedef enum
	{
		TAC_CLOCK_SELECT_LBIT = 0x01, // this is two bits !!
		TAC_CLOCK_SELECT_HBIT = (0x01 << 1),
		TAC_ENABLE = (0x01 << 2)
	} TimerControl_Flags;

	typedef enum
	{
		CLOCK_00 = 0x00,
		CLOCK_01 = 0x01,
		CLOCK_10 = 0x02,
		CLOCK_11 = 0x03
	} Clock_Select;

	#define TIMA_CLOCK_SPEED_00 1024
	#define TIMA_CLOCK_SPEED_01 16
	#define TIMA_CLOCK_SPEED_10 64
	#define TIMA_CLOCK_SPEED_11 256


	/********************************************************************************************
		PPU Specific
	*********************************************************************************************/

	#define LCD_WIDTH  160
	#define LCD_HEIGHT 144
	#define FIFO_SIZE  8

	#define OAM_CYCLES     80
	#define VBLANK_CYCLES  456
	#define HBLANK_CYCLES  204
	#define DRAWING_CYCLES 173

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
		STAT_PPU_MODE_LBIT = 0x01, // this is two bits !!
		STAT_PPU_MODE_HBIT = (0x01 << 1),
		STAT_LYC_EQUAL_LY = (0x01 << 2),
		STAT_MODE_0_INT_SELECT = (0x01 << 3),
		STAT_MODE_1_INT_SELECT = (0x01 << 4),
		STAT_MODE_2_INT_SELECT = (0x01 << 5),
		STATE_LYC_INT_SELECT = (0x01 << 6)
	} STAT_FLags;
}