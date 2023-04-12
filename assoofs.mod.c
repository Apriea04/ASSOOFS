#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0xc6a83232, "module_layout" },
	{ 0x274ed669, "generic_delete_inode" },
	{ 0xbf7903a7, "kill_block_super" },
	{ 0x374ad5ad, "unregister_filesystem" },
	{ 0x963d02f6, "register_filesystem" },
	{ 0x29945ec1, "d_make_root" },
	{ 0xb57164e0, "current_time" },
	{ 0xa0b89fad, "inode_init_owner" },
	{ 0x9c641dc9, "new_inode" },
	{ 0x294c4f13, "__brelse" },
	{ 0xe71025ce, "kmem_cache_alloc_trace" },
	{ 0xb0549baa, "kmalloc_caches" },
	{ 0xd69269d7, "__bread_gfp" },
	{ 0x45c680fe, "mount_bdev" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "C8F36BC2C48644195A44B88");
