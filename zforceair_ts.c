/* zforceair_ts.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>

static int zforceair_ts_probe(struct i2c_client *client,
                              const struct i2c_device_id *id)
{
    /* initialize device */
    /* register to a kernel framework */
    // i2c_set_clientdata(client, <private data>);
    return 0;
}
static int zforceair_ts_remove(struct i2c_client *client)
{
    // int erik = i2c_get_clientdata(client);
    /* unregister device from kernel framework */
    /* shut down the device */
    return 0;
}

static const struct i2c_device_id zforceair_ts_id[] =
    {
        {"zforceair_ts", 0},
        {}};
MODULE_DEVICE_TABLE(i2c, zforceair_ts_id);
#ifdef CONFIG_OF
static const struct of_device_id zforceair_ts_dt_ids[] =
    {
        {
            .compatible = "neonode,zforceair_ts",
        },
        {}};
MODULE_DEVICE_TABLE(of, zforceair_ts_dt_ids);
#endif
static struct i2c_driver zforceair_ts_driver =
    {
        .probe = zforceair_ts_probe,
        .remove = zforceair_ts_remove,
        .id_table = zforceair_ts_id,
        .driver = {
            .name = "zforceair_ts",
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(zforceair_ts_dt_ids),
        },
};
module_i2c_driver(zforceair_ts_driver);

// static int __init zforceair_ts_init(void)
// {
//     pr_alert("Good morrow to this fair assembly.\n");
//     return 0;
// }
// static void __exit zforceair_ts_exit(void)
// {
//     pr_alert("Alas, poor world, what treasure hast thou lost!\n");
// }
// module_init(zforceair_ts_init);
// module_exit(zforceair_ts_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Greeting module");
MODULE_AUTHOR("William Shakespeare");