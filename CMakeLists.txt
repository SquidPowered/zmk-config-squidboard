if(CONFIG_ZMK_INPUT_PROCESSOR_JOYSTICK_WASD)
  zephyr_library()
  zephyr_library_sources(src/input_processor_joystick_wasd.c)
endif()
