#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>

#include <plat/devices.h>
#include <plat/rda_debug.h>
#include <mach/board.h>

#include "rda_gouda.h"
#include "rda_panel.h"

#define ILI9806G_MCU_CHIP_ID			0x9806
#ifdef CONFIG_PM
#include <linux/regulator/consumer.h>
#include <mach/regulator.h>
#endif /* CONFIG_PM */

//static u8 vcom=0x3b;
static struct rda_lcd_info ILI9806g_mcu_info;
static struct rda_panel_id_param ILI9806g_id_param;

/* wngl, for FPGA */

#if 1//def ILI9806G_MCU_397_XCX_K_20150813
#define ILI9806G_MCU_TIMING {\
	{\
		.tas = 7,\
		.tah = 7,\
		.pwl = 15,\
		.pwh = 15\
	}\
}
#else
#define ILI9806G_MCU_TIMING {\
	{\
		.tas = 15,\
		.tah = 15,\
		.pwl = 16,\
		.pwh = 16\
	}\
}
#endif
 
#define ILI9806G_MCU_CONFIG {\
	{\
		.cs = GOUDA_LCD_CS_0,\
		.output_fmt = GOUDA_LCD_OUTPUT_FORMAT_16_bit_RGB565,\
		.cs0_polarity = false,\
		.cs1_polarity = false,\
		.rs_polarity = false,\
		.wr_polarity = false,\
		.rd_polarity = false,\
		.te_en       =   1,\
		.tecon2	     = 0x100\
	}\
}

#define delayms(_ms_) msleep(_ms_)

static struct rda_lcd_info ILI9806g_mcu_info;

static int ILI9806g_mcu_init_gpio(void)
{

	gpio_request(GPIO_LCD_RESET, "lcd reset");
	gpio_direction_output(GPIO_LCD_RESET, 1);
	mdelay(1);
	gpio_set_value(GPIO_LCD_RESET, 0);
	mdelay(15);
	gpio_set_value(GPIO_LCD_RESET, 1);
	mdelay(10);

	return 0;
}

static int ili9806g_mcu_readid_sub(void)
{
	u16 data[6] = {0};
	u32 cmd = 0xd3;

	//LCD_MCU_CMD(0x11);
	//mdelay(20);
	/* read id */
	//while(1) 
//{ 
	LCD_MCU_CMD(0xD3);
	panel_mcu_read_id(cmd,&ILI9806g_id_param,data);

	printk(KERN_INFO "rda_fb: ILI9806g_mcu_lyq_20131211 ID:"
	       "%02x %02x %02x %02x %02x %02x\n",
	       data[0], data[1], data[2], data[3], data[4], data[5]);
//}
	if(data[2] == 0x98 && data[3] == 0x06)
		return 1;
	return 0;

}

static int ILI9806g_mcu_readid(void)
{
	struct regulator *lcd_reg;
	struct gouda_lcd *lcd = (void *)&(ILI9806g_mcu_info.lcd);
	int ret = 0;

#ifdef CONFIG_PM
	lcd_reg = regulator_get(NULL, LDO_LCD);
	if (IS_ERR(lcd_reg)) {
		printk(KERN_ERR"rda-fb not find lcd regulator devices\n");
		return 0;
	}

	if ( regulator_enable(lcd_reg)< 0) {
		printk(KERN_ERR"rda-fb lcd could not be enabled!\n");
		return 0;
	}
#endif /* CONFIG_PM */

	rda_gouda_pre_enable_lcd(lcd,1);
	ILI9806g_mcu_init_gpio();
	ret = ili9806g_mcu_readid_sub();
	rda_gouda_pre_enable_lcd(lcd,0);

#ifdef CONFIG_PM
	if ( regulator_disable(lcd_reg)< 0) {
		printk(KERN_ERR"rda-fb lcd could not be enabled!\n");
		//return 0;
	}
#endif /* CONFIG_PM */

	return ret;
}



static int ILI9806g_mcu_open(void)
{

	LCD_MCU_CMD(0x0001);
   
	LCD_MCU_CMD(0xff00);LCD_MCU_DATA(0x80);  // 	
	LCD_MCU_CMD(0xff01);LCD_MCU_DATA(0x09);  // enable EXTC	
	LCD_MCU_CMD(0xff02);LCD_MCU_DATA(0x01);  // 
	

	LCD_MCU_CMD(0xff80);LCD_MCU_DATA(0x80);  // enable Orise mode	
	LCD_MCU_CMD(0xff81);LCD_MCU_DATA(0x09); // 

	LCD_MCU_CMD(0xff03);LCD_MCU_DATA(0x01);  // enable SPI+I2C cmd2 read
	


//gamma DC 

	LCD_MCU_CMD(0xC0b4);LCD_MCU_DATA(0x50); //REG-pump23                                
	LCD_MCU_CMD(0xC582);LCD_MCU_DATA(0xA3);  //REG-pump23                                
	LCD_MCU_CMD(0xC590);LCD_MCU_DATA(0xd6);   //Pump setting (3x=D6)-->(2x=96)//v02 01/11	                                                
	LCD_MCU_CMD(0xC591);LCD_MCU_DATA(0xa9);  //Pump setting(VGH/VGL)     //87               
    	LCD_MCU_CMD(0xD800);LCD_MCU_DATA(0x73);  //GVDD=4.5V                                     	                                                
	LCD_MCU_CMD(0xD801);LCD_MCU_DATA(0x71);   //NGVDD=4.5V                                                                                              
	LCD_MCU_CMD(0xd900);LCD_MCU_DATA(0x73);   // VCOMDC=                                                             
								 
								                  
	LCD_MCU_CMD(0xE100);LCD_MCU_DATA(0x00);	
	LCD_MCU_CMD(0xE101);LCD_MCU_DATA(0x09);	
	LCD_MCU_CMD(0xE102);LCD_MCU_DATA(0x0f);	
	LCD_MCU_CMD(0xE103);LCD_MCU_DATA(0x0e);	
	LCD_MCU_CMD(0xE104);LCD_MCU_DATA(0x07);	
	LCD_MCU_CMD(0xE105);LCD_MCU_DATA(0x00);	
	LCD_MCU_CMD(0xE106);LCD_MCU_DATA(0x00);	
	LCD_MCU_CMD(0xE107);LCD_MCU_DATA(0x0a);	
	LCD_MCU_CMD(0xE108);	LCD_MCU_DATA(0x04);
	
	LCD_MCU_CMD(0xE109);LCD_MCU_DATA(0x07);
	LCD_MCU_CMD(0xE10a);LCD_MCU_DATA(0x0b);
	LCD_MCU_CMD(0xE10b);LCD_MCU_DATA(0x08);
	LCD_MCU_CMD(0xE10c);LCD_MCU_DATA(0x0f);
	
	LCD_MCU_CMD(0xE10d);LCD_MCU_DATA(0x10);
	
	
	LCD_MCU_CMD(0xE10e);LCD_MCU_DATA(0x0a);
	
	
	LCD_MCU_CMD(0xE10f);LCD_MCU_DATA(0x01);


 /////
	LCD_MCU_CMD(0xE200);LCD_MCU_DATA(0x00);	
	LCD_MCU_CMD(0xE201);LCD_MCU_DATA(0x09);	
	LCD_MCU_CMD(0xE202);LCD_MCU_DATA(0x0f);	
	LCD_MCU_CMD(0xE203);LCD_MCU_DATA(0x0e);	
	LCD_MCU_CMD(0xE204);LCD_MCU_DATA(0x07);	
	LCD_MCU_CMD(0xE205);LCD_MCU_DATA(0x00);	
	LCD_MCU_CMD(0xE206);LCD_MCU_DATA(0x00);	
	LCD_MCU_CMD(0xE207);LCD_MCU_DATA(0x0a);	
	LCD_MCU_CMD(0xE208);LCD_MCU_DATA(0x04);	
	LCD_MCU_CMD(0xE209);LCD_MCU_DATA(0x07);
	LCD_MCU_CMD(0xE20a);LCD_MCU_DATA(0x0b);
	LCD_MCU_CMD(0xE20b);LCD_MCU_DATA(0x08);
	LCD_MCU_CMD(0xE20c);LCD_MCU_DATA(0x0f);
	LCD_MCU_CMD(0xE20d);LCD_MCU_DATA(0x10);
	LCD_MCU_CMD(0xE20e);LCD_MCU_DATA(0x0a);	
	LCD_MCU_CMD(0xE20f);LCD_MCU_DATA(0x01);
	
	
	
	LCD_MCU_CMD(0xC181);LCD_MCU_DATA(0x55);
	LCD_MCU_CMD(0xC1a1);LCD_MCU_DATA(0x08);
	LCD_MCU_CMD(0xC0a3);LCD_MCU_DATA(0x1b);
	LCD_MCU_CMD(0xC481);LCD_MCU_DATA(0x83);
	LCD_MCU_CMD(0xC592);LCD_MCU_DATA(0x01);
	LCD_MCU_CMD(0xC5B1);LCD_MCU_DATA(0xA9);
	            
	LCD_MCU_CMD(0xCE80);LCD_MCU_DATA(0x85);
	LCD_MCU_CMD(0xCE81);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCE82);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCE83);LCD_MCU_DATA(0x84);
	LCD_MCU_CMD(0xCE84);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCE85);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCE86);LCD_MCU_DATA(0x83);
	LCD_MCU_CMD(0xCE87);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCE88);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCE89);LCD_MCU_DATA(0x82);
	LCD_MCU_CMD(0xCE8a);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCE8b);LCD_MCU_DATA(0x00);
	            
	LCD_MCU_CMD(0xCEa0);LCD_MCU_DATA(0x38);
	LCD_MCU_CMD(0xCEa1);LCD_MCU_DATA(0x02);
	LCD_MCU_CMD(0xCEa2);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEa3);LCD_MCU_DATA(0x21);
	LCD_MCU_CMD(0xCEa4);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEa5);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEa6);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEa7);LCD_MCU_DATA(0x38);
	LCD_MCU_CMD(0xCEa8);LCD_MCU_DATA(0x01);
	LCD_MCU_CMD(0xCEa9);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEaa);LCD_MCU_DATA(0x22);
	LCD_MCU_CMD(0xCEab);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEac);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEad);LCD_MCU_DATA(0x00);
	            
	LCD_MCU_CMD(0xCEb0); LCD_MCU_DATA(0x38);
	LCD_MCU_CMD(0xCEb1); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEb2); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEb3); LCD_MCU_DATA(0x23);
	LCD_MCU_CMD(0xCEb4); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEb5); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEb6); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEb7); LCD_MCU_DATA(0x30);
	LCD_MCU_CMD(0xCEb8); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEb9); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEba); LCD_MCU_DATA(0x24);
	LCD_MCU_CMD(0xCEbb); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEbc); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEbd); LCD_MCU_DATA(0x00);
	             
	LCD_MCU_CMD(0xCEc0); LCD_MCU_DATA(0x30);
	LCD_MCU_CMD(0xCEc1); LCD_MCU_DATA(0x01);
	LCD_MCU_CMD(0xCEc2); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEc3); LCD_MCU_DATA(0x25);
	LCD_MCU_CMD(0xCEc4); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEc5); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEc6); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEc7); LCD_MCU_DATA(0x30);
	LCD_MCU_CMD(0xCEc8); LCD_MCU_DATA(0x02);
	LCD_MCU_CMD(0xCEc9); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEca); LCD_MCU_DATA(0x26);
	LCD_MCU_CMD(0xCEcb); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEcc); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEcd); LCD_MCU_DATA(0x00);
	            
	LCD_MCU_CMD(0xCEd0); LCD_MCU_DATA(0x30);
	LCD_MCU_CMD(0xCEd1); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEd2); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEd3); LCD_MCU_DATA(0x27);
	LCD_MCU_CMD(0xCEd4); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEd5); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEd6); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEd7); LCD_MCU_DATA(0x30);
	LCD_MCU_CMD(0xCEd8); LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCEd9); LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCEda); LCD_MCU_DATA(0x28);
	LCD_MCU_CMD(0xCEdb); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEdc); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCEdd); LCD_MCU_DATA(0x00);
	            
	LCD_MCU_CMD(0xCFc0); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc1); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc2); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc3); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc4); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc5); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc6); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc7); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc8); LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCFc9); LCD_MCU_DATA(0x00);
	            
	LCD_MCU_CMD(0xCFd0); LCD_MCU_DATA(0x00);
	            
	LCD_MCU_CMD(0xCBc0);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBc1);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBc2);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBc3);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBc4);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBc5);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBc6);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBc7);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBc8);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBc9);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBca);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBcb);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBcc);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBcd);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBce);LCD_MCU_DATA(0x00);
	                             
	LCD_MCU_CMD(0xCBd0);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd1);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd2);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd3);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd4);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd5);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd6);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd7);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd8);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBd9);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBda);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBdb);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBdc);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBdd);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCBde);LCD_MCU_DATA(0x04);
	                              
	LCD_MCU_CMD(0xCBe0);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe1);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe2);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe3);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe4);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe5);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe6);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe7);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCBe8);LCD_MCU_DATA(0x00); 
	LCD_MCU_CMD(0xCBe9);LCD_MCU_DATA(0x00); 
	                           
	LCD_MCU_CMD(0xCC80);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC81);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC82);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC83);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC84);LCD_MCU_DATA(0x0C);
	LCD_MCU_CMD(0xCC85);LCD_MCU_DATA(0x0A);
	LCD_MCU_CMD(0xCC86);LCD_MCU_DATA(0x10);
	LCD_MCU_CMD(0xCC87);LCD_MCU_DATA(0x0E);
	LCD_MCU_CMD(0xCC88);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCC89);LCD_MCU_DATA(0x04);
	                            
	LCD_MCU_CMD(0xCC90);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC91);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC92);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC93);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC94);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC95);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC96);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC97);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC98);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC99);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC9a);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC9b);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC9c);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC9d);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCC9e);LCD_MCU_DATA(0x0B);
	                          
	LCD_MCU_CMD(0xCCa0);LCD_MCU_DATA(0x09);
	LCD_MCU_CMD(0xCCa1);LCD_MCU_DATA(0x0F);
	LCD_MCU_CMD(0xCCa2);LCD_MCU_DATA(0x0D);
	LCD_MCU_CMD(0xCCa3);LCD_MCU_DATA(0x01);
	LCD_MCU_CMD(0xCCa4);LCD_MCU_DATA(0x02);
	LCD_MCU_CMD(0xCCa5);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCa6);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCa7);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCa8);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCa9);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCaa);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCab);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCac);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCad);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCae);LCD_MCU_DATA(0x00);
	                          
	LCD_MCU_CMD(0xCCb0);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCb1);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCb2);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCb3);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCb4);LCD_MCU_DATA(0x0D);
	LCD_MCU_CMD(0xCCb5);LCD_MCU_DATA(0x0F);
	LCD_MCU_CMD(0xCCb6);LCD_MCU_DATA(0x09);
	LCD_MCU_CMD(0xCCb7);LCD_MCU_DATA(0x0B);
	LCD_MCU_CMD(0xCCb8);LCD_MCU_DATA(0x02);
	LCD_MCU_CMD(0xCCb9);LCD_MCU_DATA(0x01);
	                         
	LCD_MCU_CMD(0xCCc0);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc1);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc2);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc3);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc4);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc5);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc6);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc7);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc8);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCc9);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCca);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCcb);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCcc);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCcd);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCce);LCD_MCU_DATA(0x0E);
	                        
	LCD_MCU_CMD(0xCCd0);LCD_MCU_DATA(0x10);
	LCD_MCU_CMD(0xCCd1);LCD_MCU_DATA(0x0A);
	LCD_MCU_CMD(0xCCd2);LCD_MCU_DATA(0x0C);
	LCD_MCU_CMD(0xCCd3);LCD_MCU_DATA(0x04);
	LCD_MCU_CMD(0xCCd4);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0xCCd5);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCd6);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCd7);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCd8);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCd9);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCda);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCdb);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCdc);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCdd);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0xCCde);LCD_MCU_DATA(0x00);
	
	
	//LCD_MCU_CMD(0xff00);LCD_MCU_DATA(0xff);
	//LCD_MCU_CMD(0xff01);LCD_MCU_DATA(0xff);
	//LCD_MCU_CMD(0xff02);LCD_MCU_DATA(0xff);
	
	
	LCD_MCU_CMD(0x3600);LCD_MCU_DATA(0xC0);// Display Direction 0	  //c0
	LCD_MCU_CMD(0x3500);LCD_MCU_DATA(0x00);//TE psw add
	

	#ifdef LCD_BACKLIGHT_CONTROL_MODE
	LCD_MCU_CMD(0x5100);LCD_MCU_DATA(0xFF);// Backlight Level Control
	LCD_MCU_CMD(0x5300);LCD_MCU_DATA(0x2C);// Backlight On
	LCD_MCU_CMD(0x5500);LCD_MCU_DATA(0x00);// CABC Function Off
	#endif

	LCD_MCU_CMD(0x3A00);LCD_MCU_DATA(0x55);
	LCD_MCU_CMD(0x1100);
	mdelay(150);
	LCD_MCU_CMD(0x2900);
        mdelay(150);

	LCD_MCU_CMD(0x2A00);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0x2A01);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0x2A02);LCD_MCU_DATA(0x01);
	LCD_MCU_CMD(0x2A03);LCD_MCU_DATA(0xDF);

	LCD_MCU_CMD(0x2B00);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0x2B01);LCD_MCU_DATA(0x00);
	LCD_MCU_CMD(0x2B02);LCD_MCU_DATA(0x03);
	LCD_MCU_CMD(0x2B03);LCD_MCU_DATA(0x1F);
	LCD_MCU_CMD(0x2C00);

	return 0;
}




static int ILI9806g_mcu_sleep(void)
{

	return 0;
}



static int ILI9806g_mcu_wakeup(void)
{
	gpio_set_value(GPIO_LCD_RESET, 1);
	mdelay(1);
	gpio_set_value(GPIO_LCD_RESET, 0);
	mdelay(15);
	gpio_set_value(GPIO_LCD_RESET, 1);
	mdelay(10);
	ILI9806g_mcu_open();

	return 0;
}



static int ILI9806g_mcu_set_active_win(struct gouda_rect *r)
{
	return 0;
}



static int ILI9806g_mcu_set_rotation(int rotate)
{
	return 0;
}

static int ILI9806g_mcu_close(void)
{
	return 0;
}

static struct rda_panel_id_param ILI9806g_id_param = {
	.lcd_info = &ILI9806g_mcu_info,
	.per_read_bytes = 4,
};

static struct rda_lcd_info ILI9806g_mcu_info = {
	.ops = {
		.s_init_gpio = ILI9806g_mcu_init_gpio,
		.s_open = ILI9806g_mcu_open,
		.s_readid = ILI9806g_mcu_readid,
		.s_active_win = ILI9806g_mcu_set_active_win,
		.s_rotation = ILI9806g_mcu_set_rotation,
		.s_sleep = ILI9806g_mcu_sleep,
		.s_wakeup = ILI9806g_mcu_wakeup,
		.s_close = ILI9806g_mcu_close
	},
	.lcd = {
		.width = WVGA_LCDD_DISP_X,
		.height = WVGA_LCDD_DISP_Y,
		.lcd_interface = GOUDA_LCD_IF_DBI,
		.lcd_timing = ILI9806G_MCU_TIMING,
		.lcd_cfg = ILI9806G_MCU_CONFIG
	},
	.name = ILI9806G_MCU_PANEL_NAME,
};

/*--------------------Platform Device Probe-------------------------*/

static int rda_fb_panel_ILI9806g_mcu_probe(struct platform_device *pdev)
{
	rda_fb_register_panel(&ILI9806g_mcu_info);

	dev_info(&pdev->dev, "rda panel ILI9806g_mcu registered\n");

	return 0;
}

static int rda_fb_panel_ILI9806g_mcu_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver rda_fb_panel_ILI9806g_mcu_driver = {
	.probe = rda_fb_panel_ILI9806g_mcu_probe,
	.remove = rda_fb_panel_ILI9806g_mcu_remove,
	.driver = {
		.name = ILI9806G_MCU_PANEL_NAME
	}
};

static struct rda_panel_driver ili9806g_mcu_panel_driver = {
	.panel_type = GOUDA_LCD_IF_DBI,
	.lcd_driver_info = &ILI9806g_mcu_info,
	.pltaform_panel_driver = &rda_fb_panel_ILI9806g_mcu_driver,
};

static int __init rda_fb_panel_ILI9806g_mcu_init(void)
{
	rda_fb_probe_panel(&ILI9806g_mcu_info, &rda_fb_panel_ILI9806g_mcu_driver);
	return platform_driver_register(&rda_fb_panel_ILI9806g_mcu_driver);
}

static void __exit rda_fb_panel_ILI9806g_mcu_exit(void)
{
	platform_driver_unregister(&rda_fb_panel_ILI9806g_mcu_driver);
}

module_init(rda_fb_panel_ILI9806g_mcu_init);
module_exit(rda_fb_panel_ILI9806g_mcu_exit);
