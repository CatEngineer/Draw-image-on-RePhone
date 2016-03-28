

#include "vmlog.h"
#include "vmdcl.h"
#include "vmdcl_pwm.h"
#include "vmdcl_gpio.h"
#include "vmgraphic.h"
#include "vmgraphic_image.h"
#include "ResID.h"
#include "vmgraphic_font.h"
#include "ldisplay.h"
#include "lcd_sitronix_st7789s.h"


#define LCD_GPIO_LIGHT 3


vm_graphic_frame_t g_frame;
vm_graphic_frame_t g_rotated_frame;
const vm_graphic_frame_t* g_frame_blt_group[1];
static VMUINT8* g_font_pool;


void lcd_backlight_level(VMUINT16 ulValue)
{
	VM_DCL_HANDLE pwm_handle;
	vm_dcl_pwm_set_clock_t pwm_clock;
	vm_dcl_pwm_set_counter_threshold_t pwm_config_adv;
	vm_dcl_config_pin_mode(LCD_GPIO_LIGHT, VM_DCL_PIN_MODE_PWM);
	pwm_handle = vm_dcl_open(PIN2PWM(LCD_GPIO_LIGHT),vm_dcl_get_owner_id());
	vm_dcl_control(pwm_handle,VM_PWM_CMD_START,0);
	pwm_config_adv.counter = 100;
	pwm_config_adv.threshold = ulValue;
	pwm_clock.source_clock = 0;
	pwm_clock.source_clock_division =3;
	vm_dcl_control(pwm_handle,VM_PWM_CMD_SET_CLOCK,(void *)(&pwm_clock));
	vm_dcl_control(pwm_handle,VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD,(void *)(&pwm_config_adv));
	vm_dcl_close(pwm_handle);
}

void lcd_init()
{
	lcd_st7789s_init();
}

void lcd_init_font()
{
    VM_RESULT result;
    VMUINT32 pool_size;

    result = vm_graphic_get_font_pool_size(0, 0, 0, &pool_size);
    if(VM_IS_SUCCEEDED(result))
    {
    	g_font_pool = (VMUINT8* )vm_malloc(pool_size);
        if(NULL != g_font_pool)
        {
            vm_graphic_init_font_pool(g_font_pool, pool_size);
        }
        else
        {
            vm_log_info("allocate font pool memory failed");
        }
    }
    else
    {
        vm_log_info("get font pool size failed, result:%d", result);
    }

    g_frame.width = 240;
    g_frame.height = 240;
    g_frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
    g_frame.buffer = (VMUINT8* )vm_malloc_dma(g_frame.width * g_frame.height * 2);
    g_frame.buffer_length = (g_frame.width * g_frame.height * 2);

    g_rotated_frame.width = 240;
	g_rotated_frame.height = 240;
	g_rotated_frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
	g_rotated_frame.buffer = (VMUINT8*)vm_malloc_dma(g_rotated_frame.width * g_rotated_frame.height * 2);
	g_rotated_frame.buffer_length = (g_rotated_frame.width * g_rotated_frame.height * 2);

	//g_frame_blt_group[0] = &g_frame;
    g_frame_blt_group[0] = &g_rotated_frame;
}

void lcd_image_draw(unsigned char image_id)
{

	void* data_ptr;
	VMUINT32 size;
	vm_graphic_point_t frame_position[1] = {0, 0};

	data_ptr = vm_res_get_image(image_id, &size);
	if(data_ptr == NULL)
	{
		vm_log_info("get image failed");
		return;
	}

	vm_graphic_draw_image_memory(&g_frame,0,0,data_ptr,size,0);
	vm_graphic_rotate_frame(&g_rotated_frame, &g_frame, VM_GRAPHIC_ROTATE_180);
	vm_graphic_blt_frame((const vm_graphic_frame_t**)g_frame_blt_group, frame_position, (VMINT)1);

	vm_res_delete(image_id);
}

void lcd_screen_set(VMUINT32 BGulValue)
{
    vm_graphic_color_argb_t color;		// use to set screen and text color
    vm_graphic_point_t frame_position[1] = {0, 0};

    // set color and draw back ground
    color.a = 255;
    color.r = ((BGulValue >> 16) & 0xff);
    color.g = ((BGulValue >> 8) & 0xff);
    color.b = (BGulValue & 0xff);
    vm_graphic_set_color(color);
    vm_graphic_draw_solid_rectangle(&g_frame, 0,0, 240,240);
    vm_graphic_rotate_frame(&g_rotated_frame, &g_frame, VM_GRAPHIC_ROTATE_180);
    vm_graphic_blt_frame(g_frame_blt_group, frame_position, (VMINT)1);
}

void lcd_daw_font(VMUINT8 ulX, VMUINT8 ulY, VMINT8 str[], VMUINT32 BGulValue, VMUINT32 DTulValue)
{
	VMCHAR s[260];					// string's buffer
    vm_graphic_color_argb_t color;		// use to set screen and text color
    vm_graphic_point_t frame_position[1] = {0, 0};

    vm_chset_ascii_to_ucs2(s,260, str);
	// set color and draw back ground
    color.a = 0;
    color.r = ((BGulValue >> 16) & 0xff);
	color.g = ((BGulValue >> 8) & 0xff);
	color.b = (BGulValue & 0xff);
    vm_graphic_set_color(color);
    vm_graphic_draw_solid_rectangle(&g_frame, ulX, ulY, 240,20);

	// set color and draw text
    color.a = 0;
    color.r = ((DTulValue >> 16) & 0xff);
	color.g = ((DTulValue >> 8) & 0xff);
	color.b = (DTulValue & 0xff);
    vm_graphic_set_color(color);
    vm_graphic_set_font_size(18);
    vm_graphic_draw_text(&g_frame,ulX, ulY, (VMWSTR)s);

    vm_graphic_rotate_frame(&g_rotated_frame, &g_frame, VM_GRAPHIC_ROTATE_180);
    vm_graphic_blt_frame(g_frame_blt_group, frame_position, (VMINT)1);
}

void lcd_daw_number(VMUINT8 ulX, VMUINT8 ulY, VMUINT32 data, VMUINT32 BGulValue, VMUINT32 DTulValue)
{
	VMCHAR str[11]={0};
	VMUINT8 bitNum;
	VMUINT32 dataTemp;
	VMUINT32 dataTen;
	VMUINT8 i,j=0;

	dataTemp = data;

	while(dataTemp)
	{
		dataTemp /= 10;
		bitNum ++;
	}
	//vm_log_info("lcd_daw_number bitNum is %d", bitNum);
	str[0] = '0';
	dataTemp = data;
	while(bitNum--)
	{
		for(i=0,dataTen=1;i<bitNum;i++)dataTen *= 10;
		str[j++] = (dataTemp / dataTen) + '0';
		dataTemp %= dataTen;
	}
	//vm_log_info("lcd_daw_number is %d %d %d %d %d %d %d %d %d",str[0],str[1],str[2],str[3],str[4],str[5],str[6],str[7],str[8]);
	lcd_daw_font(ulX, ulY, str, BGulValue, DTulValue);
}

void lcd_daw_hex(VMUINT8 ulX, VMUINT8 ulY, VMUINT32 data, VMUINT32 BGulValue, VMUINT32 DTulValue)
{
	VMCHAR str[11]={0};
	VMUINT8 bitNum;
	VMUINT32 dataTemp;
	VMUINT32 dataTen;
	VMUINT8 i,j=2;

	dataTemp = data;

	while(dataTemp)
	{
		dataTemp >>= 4;
		bitNum ++;
	}

	str[0] = '0';
	str[1] = 'x';
	str[2] = '0';
	dataTemp = data;

	while(bitNum--)
	{
		for(i=0,dataTen=1;i<bitNum;i++)dataTen <<= 4;
		//vm_log_info("dataTen is %x", dataTen);
		if((dataTemp / dataTen) < 10)str[j++] = (dataTemp / dataTen) + '0';
		else str[j++] = (dataTemp / dataTen) + 'A' - 10;
		//vm_log_info("dataTemp / dataTen is %x", dataTemp / dataTen);
		dataTemp %= dataTen;
		//vm_log_info("dataTemp is %x", dataTemp);
	}
	//vm_log_info("lcd_daw_number is %d %d %d %d %d %d %d %d %d",str[0],str[1],str[2],str[3],str[4],str[5],str[6],str[7],str[8]);
	lcd_daw_font(ulX, ulY, str, BGulValue, DTulValue);
}
