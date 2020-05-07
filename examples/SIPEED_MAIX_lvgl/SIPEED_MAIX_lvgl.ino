#include <lvgl.h>
#include <Ticker.h>
#include <Sipeed_ST7789.h>

#define LVGL_TICK_PERIOD 20

SPIClass spi_(SPI0);// MUST be SPI0 for Maix series on board LCD
Ticker tick; /* timer for interrupt handler */
Sipeed_ST7789 lcd(320, 240, spi_);
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];
#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, unsigned int line, const char *fn, const char * dsc)
{
  Serial.printf("%s@%d->%s() %s\r\n", file, line, fn, dsc);
  // delay(100);
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  int32_t w = area->x2-area->x1+1;
  int32_t h = area->y2-area->y1+1;
  int32_t x,y;
  int32_t i=0;
  uint16_t* data = (uint16_t*)malloc( w*h*2 );
  uint16_t* pixels = data;

  for(y=area->y1; y<=area->y2; ++y)
  {
    for(x=area->x1; x<=area->x2; ++x)
    {
      pixels[i++]= (color_p->ch.red<<3) | (color_p->ch.blue<<8) | (color_p->ch.green>>3&0x07 | color_p->ch.green<<13);
      // or LV_COLOR_16_SWAP = 1
       ++color_p;
    }
  }
  lcd.drawImage((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)w, (uint16_t)h, data);
  free(data);
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{
  lv_tick_inc(LVGL_TICK_PERIOD);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
  static int32_t last_diff = 0;
  int32_t diff = 0; /* Dummy - no movement */
  int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */

  data->enc_diff = diff - last_diff;;
  data->state = btn_state;

  last_diff = diff;

  return false;
}

void setup() {

  Serial.begin(115200); /* prepare for possible serial debug */
  
  lcd.begin(15000000, COLOR_WHITE);
   
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  lv_init();

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the touch pad*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_ENCODER;
  indev_drv.read_cb = read_encoder;
  lv_indev_drv_register(&indev_drv);

  /*Initialize the graphics library's tick*/
  tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

  /* Create simple label */
  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label, "Hello Maixduino! (V7.0)");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}


void loop() {
  lv_task_handler(); /* let the GUI do its work */
  delay(5);
}