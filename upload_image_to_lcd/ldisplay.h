

#ifndef _LDISPLAY_H
#define _LDISPLAY_H


void lcd_init();
void lcd_init_font();
void lcd_backlight_level(VMUINT16 ulValue);
void lcd_screen_set(VMUINT32 BGulValue);
void lcd_image_draw(unsigned char image_id);
void lcd_daw_font(VMUINT8 ulX, VMUINT8 ulY, VMINT8 str[], VMUINT32 BGulValue, VMUINT32 DTulValue);
void lcd_daw_number(VMUINT8 ulX, VMUINT8 ulY, VMUINT32 data, VMUINT32 BGulValue, VMUINT32 DTulValue);
void lcd_daw_hex(VMUINT8 ulX, VMUINT8 ulY, VMUINT32 data, VMUINT32 BGulValue, VMUINT32 DTulValue);


#endif
