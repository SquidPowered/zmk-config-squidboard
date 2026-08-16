#define DT_DRV_COMPAT zmk_input_processor_joystick_wasd

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>

#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/virtual_key_position.h>

enum axis_index {
    AXIS_X,
    AXIS_Y,
    AXIS_COUNT,
};

enum binding_index {
    BINDING_UP,
    BINDING_DOWN,
    BINDING_LEFT,
    BINDING_RIGHT,
};

struct joystick_wasd_axis_state {
    struct k_work_delayable release_work;
    const struct device *dev;
    int8_t direction;
    uint8_t axis;
};

struct joystick_wasd_data {
    struct joystick_wasd_axis_state axes[AXIS_COUNT];
    uint8_t input_device_index;
};

struct joystick_wasd_config {
    uint8_t index;
    uint16_t threshold;
    uint16_t release_after_ms;
    struct zmk_behavior_binding bindings[4];
};

static uint8_t binding_for_direction(uint8_t axis, int8_t direction) {
    if (axis == AXIS_X) {
        return direction < 0 ? BINDING_LEFT : BINDING_RIGHT;
    }

    return direction < 0 ? BINDING_UP : BINDING_DOWN;
}

static void invoke_direction(const struct device *dev, uint8_t axis, int8_t direction,
                             bool pressed) {
    const struct joystick_wasd_config *config = dev->config;
    struct joystick_wasd_data *data = dev->data;
    struct zmk_behavior_binding_event event = {
        .position = ZMK_VIRTUAL_KEY_POSITION_BEHAVIOR_INPUT_PROCESSOR(data->input_device_index,
                                                                      config->index),
        .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
    };

    zmk_behavior_invoke_binding(&config->bindings[binding_for_direction(axis, direction)], event,
                                pressed);
}

static void release_axis(struct k_work *work) {
    struct k_work_delayable *delayable = k_work_delayable_from_work(work);
    struct joystick_wasd_axis_state *axis =
        CONTAINER_OF(delayable, struct joystick_wasd_axis_state, release_work);

    if (axis->direction != 0) {
        invoke_direction(axis->dev, axis->axis, axis->direction, false);
        axis->direction = 0;
    }
}

static int joystick_wasd_handle_event(const struct device *dev, struct input_event *event,
                                      uint32_t param1, uint32_t param2,
                                      struct zmk_input_processor_state *processor_state) {
    const struct joystick_wasd_config *config = dev->config;
    struct joystick_wasd_data *data = dev->data;

    if (event->type != INPUT_EV_REL ||
        (event->code != INPUT_REL_X && event->code != INPUT_REL_Y)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    uint8_t axis_index = event->code == INPUT_REL_X ? AXIS_X : AXIS_Y;
    struct joystick_wasd_axis_state *axis = &data->axes[axis_index];
    int8_t direction = 0;

    data->input_device_index = processor_state->input_device_index;

    if (event->value <= -(int32_t)config->threshold) {
        direction = -1;
    } else if (event->value >= config->threshold) {
        direction = 1;
    }

    if (direction != axis->direction) {
        if (axis->direction != 0) {
            invoke_direction(dev, axis_index, axis->direction, false);
        }
        if (direction != 0) {
            invoke_direction(dev, axis_index, direction, true);
        }
        axis->direction = direction;
    }

    if (direction != 0) {
        k_work_reschedule(&axis->release_work, K_MSEC(config->release_after_ms));
    } else {
        k_work_cancel_delayable(&axis->release_work);
    }

    return ZMK_INPUT_PROC_STOP;
}

static int joystick_wasd_init(const struct device *dev) {
    struct joystick_wasd_data *data = dev->data;

    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        data->axes[i].dev = dev;
        data->axes[i].axis = i;
        k_work_init_delayable(&data->axes[i].release_work, release_axis);
    }

    return 0;
}

static const struct zmk_input_processor_driver_api joystick_wasd_driver_api = {
    .handle_event = joystick_wasd_handle_event,
};

#define JOYSTICK_WASD_INST(n)                                                                     \
    static struct joystick_wasd_data joystick_wasd_data_##n;                                      \
    static const struct joystick_wasd_config joystick_wasd_config_##n = {                          \
        .index = n,                                                                                \
        .threshold = DT_INST_PROP(n, threshold),                                                   \
        .release_after_ms = DT_INST_PROP(n, release_after_ms),                                     \
        .bindings = {LISTIFY(4, ZMK_KEYMAP_EXTRACT_BINDING, (,), DT_DRV_INST(n))},                 \
    };                                                                                             \
    DEVICE_DT_INST_DEFINE(n, joystick_wasd_init, NULL, &joystick_wasd_data_##n,                    \
                          &joystick_wasd_config_##n, POST_KERNEL,                                  \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &joystick_wasd_driver_api);

DT_INST_FOREACH_STATUS_OKAY(JOYSTICK_WASD_INST)
