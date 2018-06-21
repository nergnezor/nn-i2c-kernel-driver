#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

static irqreturn_t zforceair_irq(int irq, void *dev_id)
{
    // struct icn8505_data *icn8505 = dev_id;
    // struct device *dev = &icn8505->client->dev;
    // struct icn8505_touch_data touch_data;
    // int i, error;

    // error = icn8505_read_data(icn8505, ICN8505_REG_TOUCHDATA,
    // 			  &touch_data, sizeof(touch_data));
    // if (error) {
    // 	dev_err(dev, "Error reading touch data: %d\n", error);
    // 	return IRQ_HANDLED;
    // }

    // if (touch_data.touch_count > ICN8505_MAX_TOUCHES) {
    // 	dev_warn(dev, "Too many touches %d > %d\n",
    // 		 touch_data.touch_count, ICN8505_MAX_TOUCHES);
    // 	touch_data.touch_count = ICN8505_MAX_TOUCHES;
    // }

    // for (i = 0; i < touch_data.touch_count; i++) {
    // 	struct icn8505_touch *touch = &touch_data.touches[i];
    // 	bool act = icn8505_touch_active(touch->event);

    // 	input_mt_slot(icn8505->input, touch->slot);
    // 	input_mt_report_slot_state(icn8505->input, MT_TOOL_FINGER, act);
    // 	if (!act)
    // 		continue;

    // 	touchscreen_report_pos(icn8505->input, &icn8505->prop,
    // 			       get_unaligned_le16(touch->x),
    // 			       get_unaligned_le16(touch->y),
    // 			       true);
    // }

    // input_mt_sync_frame(icn8505->input);
    // input_report_key(icn8505->input, KEY_LEFTMETA,
    // 		 touch_data.softbutton == 1);
    // input_sync(icn8505->input);
    pr_alert("WFWEFPJISERGOUJUJWE[ORFWF");

    return IRQ_HANDLED;
}

static int zforceair_probe(struct i2c_client *client,
                           const struct i2c_device_id *id)
{
    /* initialize device */
    /* register to a kernel framework */
    // i2c_set_clientdata(client, <private data>);
    struct device *dev = &client->dev;
    char buf[20];
    int ret, error;
    error = devm_request_threaded_irq(dev, client->irq, NULL, zforceair_irq,
                                      IRQF_ONESHOT, client->name, NULL);
    if (error)
    {
        dev_err(dev, "Error requesting irq: %d\n", error);
        return error;
    }
    pr_alert("irq: %d\n", client->irq);
    ret = i2c_master_recv(client, buf, 2);
    pr_alert("read %d bytes: %pM\n", ret, buf);
    ret = i2c_master_recv(client, buf, buf[1]);
    pr_alert("read %d bytes: %pM\n", ret, buf);
    return 0;
}
static int zforceair_remove(struct i2c_client *client)
{
    // int erik = i2c_get_clientdata(client);
    /* unregister device from kernel framework */
    pr_alert("Removed zforceAir\n");
    /* shut down the device */
    return 0;
}

static const struct i2c_device_id zforceair_id[] =
    {
        {"zforceair", 0},
        {}};
MODULE_DEVICE_TABLE(i2c, zforceair_id);
#ifdef CONFIG_OF
static const struct of_device_id zforceair_dt_ids[] =
    {
        {
            .compatible = "neonode,zforceair",
        },
        {}};
MODULE_DEVICE_TABLE(of, zforceair_dt_ids);
#endif
static struct i2c_driver zforceair_driver =
    {
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