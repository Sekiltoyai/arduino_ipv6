
#include "platform.h"
#include "tests.h"

void setup() {
  serial_init();
  serial_debug("Initializing IP6 test");

  tests_init();
}

void loop() {
  uint8_t test_id = 0;
  uint8_t verdict = 0;

  serial_debug("Waiting for next test");
  while (test_id == 0) {
    test_id = serial_wait_for_signal(1000);
  }

  verdict = tests_exec(test_id);
  if (verdict == VERDICT_OK) {
    serial_debug("OK");
  } else if (verdict == VERDICT_NOK) {
    serial_debug("NOK");
  }

  delay(100);
  serial_signal(verdict);
}
