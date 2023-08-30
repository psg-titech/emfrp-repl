/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/30
 ------------------------------------------- */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <emfrp.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/console/console.h>

static const int COL1 = 28;  // ALWAYS LOW
static const int COL2 = 11;  // ALWAYS LOW
static const int COL3 = 31;  // ALWAYS LOW
static const int COL4 = 5;   // GPIO_1!! ALWAYS LOW
static const int COL5 = 30;  // ALWAYS LOW

static const int ROW1 = 21;  // HIGH to blink
static const int ROW2 = 22;  // ALWAYS LOW
static const int ROW3 = 15;  // ALWAYS LOW
static const int ROW4 = 24;  // ALWAYS LOW
static const int ROW5 = 19;  // ALWAYS LOW
static const int BTNA = 14;  // Button A

em_object_t * objectFalse;
emfrp_t *     emfrp;

#define GPIO0 DT_NODELABEL(gpio0)
#define GPIO1 DT_NODELABEL(gpio1)

const struct device *       gpio0Dev = DEVICE_DT_GET(GPIO0);
const struct device *       gpio1Dev = DEVICE_DT_GET(GPIO1);
const struct device * const uartDev  = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

struct k_timer updateTimer;

void
setLed0(em_object_t * obj)
{
  gpio_pin_set(gpio0Dev, ROW1, obj == objectFalse ? 0 : 1);
}

void
setupLED()
{
  gpio_pin_configure(gpio0Dev, ROW1, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, ROW2, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, ROW3, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, ROW4, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, ROW5, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, COL1, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, COL2, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, COL3, GPIO_OUTPUT);
  gpio_pin_configure(gpio1Dev, COL4, GPIO_OUTPUT);
  gpio_pin_configure(gpio0Dev, COL5, GPIO_OUTPUT);
  gpio_pin_set(gpio0Dev, ROW1, 0);
  gpio_pin_set(gpio0Dev, ROW2, 0);
  gpio_pin_set(gpio0Dev, ROW3, 0);
  gpio_pin_set(gpio0Dev, ROW4, 0);
  gpio_pin_set(gpio0Dev, ROW5, 0);
  gpio_pin_set(gpio0Dev, COL1, 0);
  gpio_pin_set(gpio0Dev, COL2, 0);
  gpio_pin_set(gpio0Dev, COL3, 0);
  gpio_pin_set(gpio1Dev, COL4, 0);
  gpio_pin_set(gpio0Dev, COL5, 0);
  // register led0.
  emfrp_add_output_node(emfrp, "led0", setLed0);
}

em_object_t *
readBtnA()
{
  return gpio_pin_get(gpio0Dev, BTNA) ? emfrp_get_true_object() : emfrp_get_false_object();
}

void
setupBtn()
{
  gpio_pin_configure(gpio0Dev, BTNA, GPIO_INPUT);
  emfrp_add_input_node(emfrp, "btnA", readBtnA);
}

void
updateFunc()
{
  emfrp_update(emfrp);
}

void
setupTimer()
{
  k_timer_init(&updateTimer, updateFunc, NULL);
  k_timer_start(&updateTimer, K_MSEC(33), K_MSEC(33));
}

char buf[1024];
int  bufPos;
int
setupUart()
{
  if(!device_is_ready(uartDev)) {
    puts("UART device not found!");
    return 1;
  }
  bufPos = 0;
  return 0;
}

char *
readLine()
{
  unsigned char ch  = 0;
  int           res = 0;
  if(res = uart_poll_in(uartDev, &ch)) {
    if(res == -EBUSY)
      printf("EBUSY\n");
    else if(res != -1)
      printf("ERR: %d\n", res);
    return NULL;
  }
  putchar(ch);
  if(ch == '\n' || ch == '\r') {
    if(bufPos == 0) return NULL;
    buf[bufPos] = 0;
    bufPos      = 0;
    puts("");
    return buf;
  }
  if(ch == '\b') {
    if(bufPos == 0) return NULL;
    buf[bufPos - 1] = 0;
    bufPos--;
    return NULL;
  }
  buf[bufPos] = ch;
  bufPos++;
  if(bufPos == 1023) {
    buf[bufPos] = 0;
    bufPos      = 0;
    return buf;
  }
  return NULL;
}

void
main()
{
  console_getline_init();
  emfrp = NULL;
  em_result res;
  if((res = emfrp_create(&emfrp)) != EM_RESULT_OK) {
    puts(EM_RESULT_STR_TABLE[res]);
    puts("Emfrp creation failure.");
    while(true) {
      k_usleep(100000000);
    }
  }
  objectFalse = emfrp_get_false_object();
  setupLED();
  setupBtn();
  setupTimer();
  if(setupUart()) { puts("UART Device Failure."); }
  puts("Emfrp-REPL on Micro:bit V2.");
  while(true) {
    printf("> ");
    char * buf = console_getline();  //readLine();
    if(buf != NULL) {
      em_object_t * result = NULL;
      k_timer_stop(&updateTimer);
      em_result status = emfrp_repl(emfrp, buf, &result);
      puts(EM_RESULT_STR_TABLE[status]);
      if(status == EM_RESULT_OK) {
        emfrp_print_object(result); puts("");
      }
      k_timer_start(&updateTimer, K_MSEC(33), K_MSEC(33));
      puts("");
    } else {
      k_usleep(16 * 1000);
    }
  }
  while(true) {
    k_usleep(100000000);
  }
}
