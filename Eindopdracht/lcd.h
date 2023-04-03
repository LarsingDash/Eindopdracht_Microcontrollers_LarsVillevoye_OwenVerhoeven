#ifndef LCD_H_
#define LCD_H_

#define LCD_E 	3
#define LCD_RS	2

void writeInitCommand_strobe_e(unsigned char byte);
void write2NibbleCommand_strobe_e(unsigned char byte);
void write2NibbleData_Strobe_E(unsigned char byte);
void init(void);
void display_text(char *str);
void set_cursor(int position);
void lcd_clear();

#endif