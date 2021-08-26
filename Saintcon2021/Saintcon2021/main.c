#include "main.h"
#include <atmel_start.h>
#include "ILI9331.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

volatile uint8_t measurement_done_touch;
uint8_t  scroller_status   = 0;
uint16_t scroller_position = 0;
badgestate g_state;

#include "FrameBuffer.h"
#include "flash.h"
#include "eeprom.h"

extern uint16_t bird_raw[];

static void back_button_pressed(void);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	//touch init is disabled in this function because it's causing problems. Needs work.
	atmel_start_init();
	SysTick_Config(48000000/1000);

	/* Replace with your application code */
	pwm_set_parameters(&PWM_0, 10000, 5000);
	
	
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
	
	//Testing Flash 
	flash_init();
	//flash_read_id(flash_id);
	//flash_erase_all();
	
	//(copy bird to flash if not already there)
	uint32_t addr = 0x7f8000;
	uint16_t buf[80] = {0x80};
	flash_read(addr, buf, 80*sizeof(uint16_t));
	if (memcmp(buf, bird_raw, 80*sizeof(uint16_t))) {
		flash_erase_halfblock(addr);
		for (int i=0; i<100; ++i) {
			uint32_t offset = i * 0x100;
			flash_write(addr + offset, (uint8_t*)bird_raw + offset, 0x100);
		}
	}
	
	ext_irq_register(PIN_PA27, back_button_pressed);
	
	int x=25,y=0,dx=1,dy=1;
	uint16_t c;
	uint32_t draw_ms=0, blt_ms=0, sys_ms=0, now, tsstep=millis();
	while (1) {
		char lines[4][20];
		touch_process();
		/*if (measurement_done_touch == 1) {
			measurement_done_touch = 0;
			touch_status_display();
		}*/
		scroller_status   = get_scroller_state(0);
		scroller_position = get_scroller_position(0);
		//touchWheel = getTouchWheelPostion();
		static int x1=0,x2=0,y1=0,y2=0;
		double angle = (((double)(scroller_position)/256)*2*M_PI) + (1.5*M_PI);
		//LCD_DrawLine(x1,y1,x2,y2,RGB(100,80,100));
		x1 = 80*cos(angle)+120;
		y1 = 80*sin(angle)+120;
		x2 = 100*cos(angle)+120;
		y2 = 100*sin(angle)+120;
		//LCD_DrawLine(x1,y1,x2,y2,0xFFFF);
		
		c = dy>0?RGB(20,20,200):RGB(200,20,20);
		snprintf(lines[0], 20, "draw:%3lums", draw_ms);
		snprintf(lines[1], 20, "blt: %3lums", blt_ms);
		snprintf(lines[2], 20, "sys: %3lums", sys_ms);
		snprintf(lines[3], 20, "%3lufps", 1000/(sys_ms+blt_ms+draw_ms));
		
		
		now = millis();
		sys_ms = now - tsstep;
		tsstep = now;
		canvas_clearScreen(c);
		canvas_drawImage_FromFlash(x, y, 160, 80, addr);
		canvas_drawText(80, 85, lines[0], RGB(255,255,255));
		canvas_drawText(80, 105, lines[1], RGB(255,255,255));
		canvas_drawText(80, 125, lines[2], RGB(255,255,255));
		canvas_drawText(90, 145, lines[3], RGB(255,255,255));
		//canvas_fillRect(80,80,40,40,c);
		canvas_drawLine(x1, y1, x2, y2, scroller_status?0xFFFF:0);
		now = millis();
		draw_ms = now - tsstep;
		tsstep = now;
		canvas_blt();
		now = millis();
		blt_ms = now - tsstep;
		tsstep = now;
		
		x+=dx; y+=dy;
		if ((x<=0) || x>=80) dx*=-1;
		if ((y<=0) || y>=160) dy*=-1;

		cdcd_loop();
	}
}

bool led_toggle=false;
static void back_button_pressed(void)
{
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

