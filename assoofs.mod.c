#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
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
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0xffe053e9, "__bread_gfp" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x8d2a6f74, "__brelse" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x64485f0d, "mount_bdev" },
	{ 0x23a546f3, "mark_buffer_dirty" },
	{ 0x7c839eec, "sync_dirty_buffer" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x631837b1, "new_inode" },
	{ 0x64abbdc6, "current_time" },
	{ 0xf301d0c, "kmalloc_caches" },
	{ 0x35789eee, "kmem_cache_alloc_trace" },
	{ 0xebdee8ef, "inode_init_owner" },
	{ 0x794f41bf, "d_add" },
	{ 0x754d539c, "strlen" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x71cbab8b, "d_make_root" },
	{ 0xb67a0943, "register_filesystem" },
	{ 0xb7d65fec, "unregister_filesystem" },
	{ 0x3f13f8ed, "kill_block_super" },
	{ 0x4a00c11b, "generic_delete_inode" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "3E4AC7CB2C1C01A764B8526");
