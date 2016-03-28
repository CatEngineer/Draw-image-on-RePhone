#include "vmsystem.h" 
#include "vmlog.h"
#include "vmcmd.h"
#include "vmtimer.h"
#include "ldisplay.h"
#include "ldlcgpio.h"
#include "stdio.h"
#include "ResID.h"
#include "upload_image_to_lcd.h"

#define RLED_GPIO 	17
#define GLED_GPIO 	15
#define BLED_GPIO 	12


VM_TIMER_ID_PRECISE sys_timer_id = 0;
unsigned char sys_blink = 0;
unsigned char pic_number = 1;

void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    if(sys_blink)digitalWrite(GLED_GPIO, 1);
    else digitalWrite(GLED_GPIO, 0);
    sys_blink = !sys_blink;


    lcd_image_draw(pic_number++);
    if(pic_number > 2)pic_number = 1;

}

void sys_gpio_init(void)
{
	pinMode(RLED_GPIO,OUTPUT);
	pinMode(GLED_GPIO,OUTPUT);
	pinMode(BLED_GPIO,OUTPUT);

	digitalWrite(RLED_GPIO, 0);
	digitalWrite(GLED_GPIO, 1);
	digitalWrite(BLED_GPIO, 0);
}

void handle_sysevt(VMINT message, VMINT param)
{
    switch (message) {
        case VM_EVENT_CREATE:
        	vm_res_init(0);
        	lcd_backlight_level(50);

            sys_timer_id = vm_timer_create_non_precise(1000, sys_timer_callback, NULL);


            break;
        case VM_EVENT_PAINT:
        	lcd_screen_set(0);
            digitalWrite(RLED_GPIO, 1);
            digitalWrite(BLED_GPIO, 1);



            break;
        case VM_EVENT_QUIT:
        	vm_res_release();
            break;
    }
}

void vm_main(void) 
{
	sys_gpio_init();
	lcd_init();
	lcd_init_font();
    vm_pmng_register_system_event_callback(handle_sysevt);
}
