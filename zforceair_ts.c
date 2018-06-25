#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define DISPLAY_OFFSET_X 0
#define N_TOUCHES_MAX 5
#define N_TOUCHES_HANDLED_MAX 3
#define ASN_APPLICATION 0x40
#define ASN_CONSTRUCTED 0x20
#define X 0
#define Y 1

typedef enum {
    Neonode_Notification_Touch = 0,
    Neonode_BootComplete = 3,
    Neonode_Enable = 5,
    Neonode_OperationMode = 7,
    Neonode_Request = 14,
    Neonode_Response,
    Neonode_Notification,
    Neonode_DeviceConfiguration = 19,
} NeonodeProtocol;

typedef enum {
    Universal = 0x00,
    Application = 0x40,
    ContectSpecific = 0x80,
    Private = 0xC0,
    Constructed = 0x20,
} Asn1;

typedef enum {
    Gesture_None,
    Gesture_Rotate,
    Gesture_Zoom,
    Gesture_Pinch,
    Gesture_VerticalSwipe,
} Gesture;

typedef struct
{
    int velocity[2];
    int counter;
    uint16_t location[2];
    uint16_t lastStationaryLocation[2];
    uint16_t width;
    uint8_t id;
    uint8_t event;
} Touch;

typedef struct
{
    Touch touch[N_TOUCHES_MAX];
    Gesture gesture;
    int gestureValue;
    uint16_t center[2];
    uint16_t center_start[2];
    uint16_t radius;
    uint16_t radius_start;
    uint16_t xMax;
    uint16_t yMax;
    uint8_t count;
} Touches;

typedef enum {
    TouchEventDown = 0,
    TouchEventMove = 1,
    TouchEventUp = 2,
    touchPacketSizeI = 7,
    IdIndex = 10,
    touchLength = 11,
} TouchProtocol;

static Touches touches;

// static u8 zForceSetNTouches[] = {0xEE, 0x0B, 0xEE, 0x09, 0x40, 0x02, 0x02, 0x00, 0x73, 0x03, 0x86, 0x01, N_TOUCHES_MAX};
static u8 zForceEnable[] = { 0xEE, 0x0A, 0xEE, 0x08, 0x40, 0x02, 0x02, 0x00, 0x65, 0x02, 0x81, 0x00 };
// static u8 zForceOperationMode[] = {0xEE, 0x0E, 0xEE, 0x0C, 0x40, 0x02, 0x02, 0x00, 0x67, 0x06, 0x80, 0x01, 0xFF, 0x83, 0x01, 0xFF};

struct zforceair {
    struct i2c_client* client;
    struct input_dev* input;
    struct gpio_desc* wake_gpio;
    // struct touchscreen_properties prop;
    char firmware_name[32];
};

static int zforceair_write(struct i2c_client* client, u8 data[], u8 length)
{
    pr_alert("Sending %d bytes: %pM\n", length, data);

    i2c_master_send(client, data, length);

    return 0;
}
static int zforceair_read(struct zforceair* zforceair)
{
    struct i2c_client* client = zforceair->client;
    struct input_dev* input = zforceair->input;

    char buf[200];
    int header, subtype;
    Touch *touch, *next;
    u16 locationPrevious[2];

    int ret = i2c_master_recv(client, buf, 2);
    // pr_alert("read %d bytes: %pM\n", ret, buf);
    if (buf[0] != 0xEE)
        return -EIO;
    ret = i2c_master_recv(client, buf, buf[1]);
    // else
    // ret = i2c_master_recv(client, buf, 2);
    // pr_alert("read %d bytes: %pM\n", ret, buf);

    header = buf[0] & 0x1f; /* 5 bits */
    subtype = buf[6] & 0x1f;
    if (header == Neonode_Notification) {
        if (subtype == Neonode_BootComplete) {
            // HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS, zForceSetNTouches, sizeof(zForceSetNTouches), I2C_TIMEOUT);
            pr_alert("Boot complete\n");

            zforceair_write(client, zForceEnable, sizeof(zForceEnable));

        } else if (subtype == Neonode_Notification_Touch) {
            int i;
            int count = buf[touchPacketSizeI] / touchLength;
            if (count != touches.count) {
                touches.gesture = Gesture_None;
            }
            // uint8_t *b = &buf[IdIndex];
            i = IdIndex;
            for (touch = touches.touch; touch < &touches.touch[count]; ++touch) {
                int axis, tip;
                int id = buf[i++];

                if (id != touch->id) {
                    for (next = touch + 1; next <= touches.touch + touches.count; ++next) {
                        if (next->id == id) {
                            *touch = *next;
                        }
                    }
                }
                locationPrevious[X] = touch->location[X];
                locationPrevious[Y] = touch->location[Y];
                touch->id = id;
                touch->event = buf[i++];
                for (axis = 0; axis < 2; ++axis) {
                    int velocity;
                    touch->location[axis] = buf[i] << 8 | buf[i + 1];
                    i += 2;
                    if (axis == X) {
#ifdef DISPLAY_INVERT_X
                        touch->location[X] = touches.xMax - touch->location[X];
#endif
                        touch->location[X] = max(0, touch->location[X] - DISPLAY_OFFSET_X);
                    }
                    velocity = touch->location[axis] - locationPrevious[axis];
                    touch->velocity[axis] = (7 * touch->velocity[axis] + velocity) / 8;
                }
                touch->width = (7 * touch->width + (buf[i] << 8 | buf[i + 1])) / 8;

                if (touch->event == TouchEventDown) {
                    touch->counter = 1;
                    touch->velocity[X] = 0;
                    touch->velocity[Y] = 0;
                    touches.gesture = Gesture_None;
                } else {
                    touch->counter = min(0xff, touch->counter + 1);
                }
                i += 5;
                tip = touch->event != TouchEventUp;
                input_event(input, EV_ABS, ABS_MT_SLOT, touch - touches.touch);
                if (!tip) {
                    input_event(input, EV_ABS, ABS_MT_TRACKING_ID, -1);
                } else {
                    input_event(input, EV_ABS, ABS_MT_TRACKING_ID, id);
                    input_event(input, EV_ABS, ABS_MT_POSITION_X, touch->location[X]);
                    input_event(input, EV_ABS, ABS_MT_POSITION_Y, touch->location[Y]);
                }
            }
            touches.count = count;
            pr_alert("x: %d, y: %d\n", touches.touch[0].location[X], touches.touch[0].location[Y]);
            // input_report_rel(input, REL_X, 1);
            // input_report_rel(input, REL_Y, touches.touch[0].velocity[Y]);
            // input_report_rel(input, REL_Y, 2);
            // input_report_key(input, KEY_LEFTMETA,
            // touches.touch[0].location[X] > 200);
            input_sync(input);
            // Gestures_Process();
            // Display_DrawTouch();
        }
    } else if (header == Neonode_Response) {
        if (subtype == Neonode_DeviceConfiguration) {
            // touches.xMax = (buf[18] << 8) | buf[19];
            // touches.yMax = (buf[22] << 8) | buf[23];

            // HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS, zForceOperationMode, sizeof(zForceOperationMode), I2C_TIMEOUT);
        } else if (subtype == Neonode_OperationMode) {
            // HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS, zForceEnable, sizeof(zForceEnable), I2C_TIMEOUT);
        } else if (subtype == Neonode_Enable) {
            pr_alert("zForce AIR enabled\n");
            // Display_DrawTouch();
        }
    }
    return 0;
}

static irqreturn_t zforceair_irq(int irq, void* dev_id)
{
    struct zforceair* zforceair = dev_id;
    zforceair_read(zforceair);
    // struct device *dev = &zforceair->client->dev;
    // struct zforceair_touch_data touch_data;
    // int i, error;

    // error = zforceair_read_data(zforceair, zforceair_REG_TOUCHDATA,
    // 			  &touch_data, sizeof(touch_data));
    // if (error) {
    // 	dev_err(dev, "Error reading touch data: %d\n", error);
    // 	return IRQ_HANDLED;
    // }

    // if (touch_data.touch_count > zforceair_MAX_TOUCHES) {
    // 	dev_warn(dev, "Too many touches %d > %d\n",
    // 		 touch_data.touch_count, zforceair_MAX_TOUCHES);
    // 	touch_data.touch_count = zforceair_MAX_TOUCHES;
    // }

    // for (i = 0; i < touch_data.touch_count; i++) {
    // 	struct zforceair_touch *touch = &touch_data.touches[i];
    // 	bool act = zforceair_touch_active(touch->event);

    // 	input_mt_slot(zforceair->input, touch->slot);
    // 	input_mt_report_slot_state(zforceair->input, MT_TOOL_FINGER, act);
    // 	if (!act)
    // 		continue;

    // 	touchscreen_report_pos(zforceair->input, &zforceair->prop,
    // 			       get_unaligned_le16(touch->x),
    // 			       get_unaligned_le16(touch->y),
    // 			       true);
    // }

    // input_mt_sync_frame(zforceair->input);
    // input_report_key(zforceair->input, KEY_LEFTMETA,
    // 		 touch_data.softbutton == 1);
    // input_sync(zforceair->input);
    return IRQ_HANDLED;
}

static int zforceair_probe(struct i2c_client* client, const struct i2c_device_id* id)
{
    struct device* dev = &client->dev;
    struct zforceair* zforceair;
    struct input_dev* input;
    int error;
    char buf[3];

    if (!client->irq) {
        dev_err(dev, "No irq specified\n");
        return -EINVAL;
    }

    zforceair = devm_kzalloc(dev, sizeof(*zforceair), GFP_KERNEL);
    if (!zforceair)
        return -ENOMEM;

    input = devm_input_allocate_device(dev);
    if (!input)
        return -ENOMEM;

    input->name = client->name;
    input->id.bustype = BUS_I2C;

    // input_set_capability(input, EV_REL, REL_X);
    // input_set_capability(input, EV_REL, REL_Y);
    input_set_capability(input, EV_ABS, ABS_MT_POSITION_X);
    input_set_capability(input, EV_ABS, ABS_MT_POSITION_Y);
    input_set_capability(input, EV_KEY, KEY_LEFTMETA);

    zforceair->client = client;
    zforceair->input = input;
    input_set_drvdata(input, zforceair);
    input_set_abs_params(input, ABS_MT_POSITION_X, 0, 1000, 0, 0);
    input_set_abs_params(input, ABS_MT_POSITION_Y, 0, 1000, 0, 0);

    // touchscreen_parse_properties(input, true, &icn8505->prop);
    if (!input_abs_get_max(input, ABS_MT_POSITION_X) || !input_abs_get_max(input, ABS_MT_POSITION_Y)) {
        dev_err(dev, "Error touchscreen-size-x and/or -y missing\n");
        return -EINVAL;
    }

    error = input_mt_init_slots(input, N_TOUCHES_MAX,
        INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);
    if (error)
        return error;
    error = devm_request_threaded_irq(dev, client->irq, NULL, zforceair_irq,
        IRQF_ONESHOT, client->name, zforceair);
    if (error) {
        dev_err(dev, "Error requesting irq: %d\n", error);
        return error;
    }
    /* Register the device in input subsystem */
    error = input_register_device(input);
    if (error) {
        dev_err(&client->dev,
            "Input device register failed: %d\n", error);
    }

    i2c_set_clientdata(client, zforceair);
    /* Initialize sensor */
    i2c_master_recv(client, buf, 3);

    return 0;
}
static int zforceair_remove(struct i2c_client* client)
{
    // int erik = i2c_get_clientdata(client);
    /* unregister device from kernel framework */
    struct zforceair* zforceair = i2c_get_clientdata(client);

    devm_free_irq(&client->dev, client->irq, zforceair);

    input_unregister_device(zforceair->input);
    pr_alert("Removed zforceAir\n");
    /* shut down the device */
    return 0;
}

static const struct i2c_device_id zforceair_id[] = { { "zforceair", 0 }, {} };
MODULE_DEVICE_TABLE(i2c, zforceair_id);
#ifdef CONFIG_OF
static const struct of_device_id zforceair_dt_ids[] = {
    {
        .compatible = "neonode,zforceair",
    },
    {}
};
MODULE_DEVICE_TABLE(of, zforceair_dt_ids);
#endif
static struct i2c_driver zforceair_driver = {
    .probe = zforceair_probe,
    .remove = zforceair_remove,
    .id_table = zforceair_id,
    .driver = {
        .name = "zforceair",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(zforceair_dt_ids),
    },
};
module_i2c_driver(zforceair_driver);

// static int __init zforceair_init(void)
// {
//     pr_alert("Good morrow to this fair assembly.\n");
//     return 0;
// }
// static void __exit zforceair_exit(void)
// {
//     pr_alert("Alas, poor world, what treasure hast thou lost!\n");
// }
// module_init(zforceair_init);
// module_exit(zforceair_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Greeting module");
MODULE_AUTHOR("William Shakespeare");