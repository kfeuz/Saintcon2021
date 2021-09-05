#include "main.h"
#include "ILI9331.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

volatile uint8_t measurement_done_touch;
uint8_t  scroller_status   = 0;
uint16_t scroller_position = 0;
badgestate g_state;

bool back_event = false;

#include "FrameBuffer.h"
#include "flash.h"
#include "eeprom.h"

extern uint16_t bird_raw[];

static void back_button_pressed(void);

static struct timer_task Timer_task1;
/**
 * Example of using Timer.
 */
static void Timer_task1_cb(const struct timer_task *const timer_task)
{
	touch_process();
}

void Timer_touch_init(void)
{
	Timer_task1.interval = 20;
	Timer_task1.cb       = Timer_task1_cb;
	Timer_task1.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&Timer, &Timer_task1);
	timer_start(&Timer);
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	//touch init is disabled in this function because it's causing problems. Needs work.
	atmel_start_init();
	SysTick_Config(48000000/1000);

	/* Replace with your application code */
	pwm_set_parameters(&PWM_0, 10000, 5000);
	
	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	
	
	gpio_set_pin_direction(PIN_PB17,GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PIN_PB17,true);
	LCD_Init();
	
	eeprom_init();
	
	//test eeprom
	eeprom_load_state();
	if (g_state.badge_bitmask != 0xaabbccdd) {
		g_state.badge_bitmask = 0xaabbccdd;
		eeprom_save_state();
	}
	
	flash_init();
	
	//Testing Flash
	//(copy bird to flash if not already there)
	uint16_t buf[80] = {0x80};
	flash_read(BIRD_IMG, buf, 80*sizeof(uint16_t));
	if (memcmp(buf, bird_raw, 80*sizeof(uint16_t))) {
		flash_erase_halfblock(BIRD_IMG);
		for (int i=0; i<100; ++i) {
			uint32_t offset = i * 0x100;
			flash_write(BIRD_IMG + offset, (uint8_t*)bird_raw + offset, 0x100);
		}
	}

	LCD_FillRect(0, 0, 240, 240, RGB(10,10,200));

	canvas_clearScreen(RGB(10,10,200));
	canvas_drawText(80,100, "Magic", RGB(255,255,255));
	canvas_blt();
	
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
	
	
	spi_m_sync_get_io_descriptor(&SPI_0, &io);

	spi_m_sync_enable(&SPI_0);

	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	
	//NFC Test - Remove in final code
	st25r95Initialize();
	delay_ms(1);
	if(st25r95CheckChipID()){
		canvas_drawText(80,120,"NFC: PASS",RGB(255,255,255));
		canvas_blt();
	} else {
		canvas_drawText(80,120,"NFC: FAIL", RGB(255,0,0));
		canvas_blt();
	}
	//End NFC Test
	
	NFC_init();
	
	ext_irq_register(PIN_PA27, back_button_pressed);
	Timer_touch_init();
	
	Scene scene = TEST;
	bool changed = true;

	while (1) {
		exampleRfalPollerRun(); //NFC
		//exampleNFCARun();
		touch_process();
		/*if (measurement_done_touch == 1) {
			measurement_done_touch = 0;
			touch_status_display();
		}*/
		scroller_status   = get_scroller_state(0);
		scroller_position = get_scroller_position(0);
		
		Scene ns;
		switch(scene) {
		case TEST:
			ns = test_scene_loop(changed);
			break;
		case COMBO:
			ns = combo_scene_loop(changed);
			break;
		case INVENTORY:
			ns = inventory_scene_loop(changed);
			break;
		case MACHINE:
			ns = machine_scene_loop(changed);
			break;
		case BUILD:
			ns = build_scene_loop(changed);
			break;
		case MENU:
		default:
			ns = menu_scene_loop(changed);
		}
		changed = false;
		
		if (ns != scene) {
			changed = true;
			scene = ns;
		}
		
		cdcd_loop();
	}
}

/** \brief Do SPI read in polling way
 *  For SPI master, activate CS, do send 0xFFs and read data, deactivate CS.
 *
 *  It blocks until all data read or error.
 *
 *  \param[in, out] spi Pointer to the HAL SPI instance.
 *  \param[out] buf Pointer to the buffer to store read data.
 *  \param[in] size Size of the data in number of characters.
 *  \return Operation status.
 *  \retval size Success.
 *  \retval >=0 Time out, with number of characters read.
 */
int32_t spi_m_sync_io_readwrite(struct io_descriptor *io, uint8_t *rxbuf, uint8_t *txbuf, const uint16_t length)
{
	ASSERT(io);

	struct spi_m_sync_descriptor *spi = CONTAINER_OF(io, struct spi_m_sync_descriptor, io);
	struct spi_xfer               xfer;

	xfer.rxbuf = rxbuf;
	xfer.txbuf = txbuf;
	xfer.size  = length;

	return spi_m_sync_transfer(spi, &xfer);
bool led_toggle=false;
static void back_button_pressed(void)
{
	back_event = true;
	led_toggle = !led_toggle;
	if(led_toggle){
		//gpio_set_pin_level(PIN_PA21,true);
		pwm_disable(&PWM_0);
	} else {
		//gpio_set_pin_level(PIN_PA21,false);
		pwm_enable(&PWM_0);
	}
}

volatile uint32_t MS_Timer = 0;
void SysTick_Handler(void) {
	MS_Timer++;                // Increment global millisecond timer
}

uint32_t millis() {
	return MS_Timer++;
}


int getTouchLocation() {
	int ret = scroller_position*360/256 + 270;
	if (ret > 360) ret-=360;
	return ret;
}

