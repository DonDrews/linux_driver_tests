#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xdd05b091, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xdb5ba406, "class_destroy" },
	{ 0xd9953b4f, "class_unregister" },
	{ 0x3a4ed5c8, "device_destroy" },
	{ 0xe533c10a, "device_create" },
	{ 0xa80a1f07, "__class_create" },
	{ 0xb4d27e4b, "__register_chrdev" },
	{ 0x28118cb6, "__get_user_1" },
	{ 0x9c7104ab, "try_module_get" },
	{ 0x581513d4, "module_put" },
	{ 0xbb72d4fe, "__put_user_1" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "A607F5A3E72FA1ADBCAD4BE");