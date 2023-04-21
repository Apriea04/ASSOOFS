#include <linux/module.h>      /* Needed by all modules */
#include <linux/kernel.h>      /* Needed for KERN_INFO  */
#include <linux/init.h>        /* Needed for the macros */
#include <linux/fs.h>          /* libfs stuff           */
#include <linux/buffer_head.h> /* buffer_head           */
#include <linux/slab.h>        /* kmem_cache            */
#include "assoofs.h"

MODULE_LICENSE("GPL");

struct assoofs_inode_info *assoofs_get_inode_info(struct super_block *sb, uint64_t inode_no)
{
    struct assoofs_inode_info *inode_info = NULL;
    struct buffer_head *bh;

    struct assoofs_super_block_info *afs_sb = sb->s_fs_info;
    struct assoofs_inode_info *buffer = NULL;

    int i;
    printk(KERN_INFO "assoofs accessing disk for reading inode container\n");
    // Acceder a disco para leer el bloque con el almacén de inodos
    bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER);
    inode_info = (struct assoofs_inode_info *)bh->b_data;

    // Recorrer almacén de inodos en busca del nodo inode_no:
    printk(KERN_INFO "assoofs looking for inode\n");
    for (i = 0; i < afs_sb->inodes_count; i++)
    {
        if (inode_info->inode_no == inode_no)
        {
            buffer = kmalloc(sizeof(struct assoofs_inode_info), GFP_KERNEL);
            memcpy(buffer, inode_info, sizeof(*buffer));
            break;
        }
        inode_info++;
    }

    brelse(bh);
    return buffer;
}

/*
 *  Operaciones sobre ficheros
 */
ssize_t assoofs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos);
ssize_t assoofs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos);
const struct file_operations assoofs_file_operations = {
    .read = assoofs_read,
    .write = assoofs_write,
};

ssize_t assoofs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
    printk(KERN_INFO "Read request\n");
    return 0;
}

ssize_t assoofs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
    printk(KERN_INFO "Write request\n");
    return 0;
}

/*
 *  Operaciones sobre directorios
 */
static int assoofs_iterate(struct file *filp, struct dir_context *ctx);
const struct file_operations assoofs_dir_operations = {
    .owner = THIS_MODULE,
    .iterate = assoofs_iterate,
};

static int assoofs_iterate(struct file *filp, struct dir_context *ctx)
{
    printk(KERN_INFO "Iterate request\n");
    return 0;
}

/*
 *  Operaciones sobre inodos
 */
static int assoofs_create(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static int assoofs_mkdir(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode);
static struct inode_operations assoofs_inode_ops = {
    .create = assoofs_create,
    .lookup = assoofs_lookup,
    .mkdir = assoofs_mkdir,
};

static struct inode *assoofs_get_inode(struct super_block *sb, int ino)
{
    struct assoofs_inode_info *info = assoofs_get_inode_info(sb, ino);
    struct inode *new = new_inode(sb);
    new->i_ino = ino;
    new->i_sb = sb;
    new->i_op = &assoofs_inode_ops;

    printk(KERN_INFO "Getting inode.\n");

    // Para i_fop tenemos que sabe si es un fichero o directorio:
    if (S_ISDIR(info->mode))
    {
        new->i_fop = &assoofs_dir_operations;
    }
    else if (S_ISREG(info->mode))
    {
        new->i_fop = &assoofs_file_operations;
    }
    else
    {
        printk(KERN_ERR "Unknown inode type. Neither a directory nor a file.\n");
    }

    // Para las fechas del inodo:
    new->i_atime = new->i_mtime = new->i_ctime = current_time(new);
    // Guardamos en i_private la información persistente
    new->i_private = info;

    printk(KERN_INFO "Got inode.\n");

    return new;
}

struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    struct assoofs_inode_info *parent_info;
    struct super_block *sb;
    struct buffer_head *bh;
    struct assoofs_dir_record_entry *record;
    int i;

    printk(KERN_INFO "Lookup request\n");

    // Acceder al bloque de disco con el contenido del directorio apuntado por parent_inode
    parent_info = parent_inode->i_private;
    printk(KERN_INFO "Lookup request. Borrame 1\n");
    sb = parent_inode->i_sb;
    printk(KERN_INFO "Lookup request. Borrame 2\n");
    bh = sb_bread(sb, parent_info->data_block_number);

    printk(KERN_INFO "Lookup request. Borrame\n");

    // Recorrer el contenido del directorio buscando la entrada cuyo nombre se corresponda con el que buscamos.
    // Cuando se localiza la entrada, se contruye el inodo correspondiente.
    record = (struct assoofs_dir_record_entry *)bh->b_data;
    for (i = 0; i < parent_info->dir_children_count; i++)
    {
        printk(KERN_INFO "Lookup request. Borrrame, i=%d\n", i);
        if (!strcmp(record->filename, child_dentry->d_name.name))
        {
            struct inode *inode = assoofs_get_inode(sb, record->inode_no);
            inode_init_owner(sb->s_user_ns, inode, parent_inode, ((struct assoofs_inode_info *)inode->i_private)->mode);
            d_add(child_dentry, inode);
            return NULL;
        }
        record++;
    }

    printk(KERN_ERR "No inode found\n");

    return NULL;
}

static int assoofs_create(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    struct inode *inode;
    struct super_block *sb;
    uint64_t count;

    printk(KERN_INFO "New file request\n");

    sb = dir->i_sb;                                                           // puntero al superbloque desde dir
    count = ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count; // número de inodos de la información persistente del superbloque
    inode = new_inode(sb);
    inode->i_sb = sb;
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
    inode->i_op = &assoofs_inode_ops;
    inode->i_ino = ++count; // Asignar nuevo número al inodo a parti de count
    if (count > ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED)
    {
        // TODO
    }
    else
    {
        // TODO
    }
    return 0;
}

static int assoofs_mkdir(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    printk(KERN_INFO "New directory request\n");
    return 0;
}

/*
 *  Operaciones sobre el superbloque
 */
static const struct super_operations assoofs_sops = {
    .drop_inode = generic_delete_inode,
};

/*
 *  Inicialización del superbloque
 */
int assoofs_fill_super(struct super_block *sb, void *data, int silent)
{
    // Declaraciones juntas para cumplir con ISO C90
    struct buffer_head *bh;
    struct assoofs_super_block_info *assoofs_sb;

    struct inode *root_inode;
    printk(KERN_INFO "assoofs_fill_super request\n");
    // 1.- Leer la información persistente del superbloque del dispositivo de bloques

    bh = sb_bread(sb, ASSOOFS_SUPERBLOCK_BLOCK_NUMBER);
    assoofs_sb = (struct assoofs_super_block_info *)bh->b_data;
    brelse(bh); // Liberar la memoria
    printk(KERN_INFO "assoofs_fill_super request. Liberando memoria\n");

    // 2.- Comprobar los parámetros del superbloque
    if (assoofs_sb->magic != ASSOOFS_MAGIC || assoofs_sb->block_size != ASSOOFS_DEFAULT_BLOCK_SIZE)
    {
        printk("Error with superblock parameters\n"); // TODO: completar esto
        return -1;
    }

    printk(KERN_INFO "assoofs_fill_super request. Escribiré en disco\n");
    // 3.- Escribir la información persistente leída del dispositivo de bloques en el superbloque sb, incluído el campo s_op con las operaciones que soporta.
    sb->s_magic = ASSOOFS_MAGIC;
    sb->s_maxbytes = ASSOOFS_DEFAULT_BLOCK_SIZE;
    sb->s_op = &assoofs_sops;
    sb->s_fs_info = bh; // TODO check this
    // 4.- Crear el inodo raíz y asignarle operaciones sobre inodos (i_op) y sobre directorios (i_fop)
    printk(KERN_INFO "assoofs_fill_super request. Escribí en disco\n");

    root_inode = new_inode(sb);                                 // Inicializar una variable inode
    inode_init_owner(sb->s_user_ns, root_inode, NULL, S_IFDIR); // SIFDIR para directorios, SIFREG para ficheros

    root_inode->i_ino = ASSOOFS_ROOTDIR_INODE_NUMBER;                                           // Número de inodo
    root_inode->i_sb = sb;                                                                      // Puntero al superbloque
    root_inode->i_op = &assoofs_inode_ops;                                                      // Dirección de una variable de tipo struct inode_operations previamente declarada
    root_inode->i_fop = &assoofs_dir_operations;                                                // Dirección de una variable de tipo struct flie_operations previamente declarada. En la práctica tenemos 2: assoofs_dir_operations y assoofs_file_operations. La primera la utilizaremos cuando creemos inodos para directorios (como el directorio ra´ız) y la segunda cuando creemos inodos para ficheros.
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode); // Fechas
    root_inode->i_private = assoofs_get_inode_info(sb, ASSOOFS_ROOTDIR_INODE_NUMBER);           // Información persistente del inodo

    sb->s_root = d_make_root(root_inode);
    printk(KERN_INFO "assoofs_fill_super request. Creé e hice inodo raíz\n");
    return 0;
}

/*
 *  Montaje de dispositivos assoofs
 */
static struct dentry *assoofs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    struct dentry *ret;
    printk(KERN_INFO "assoofs_mount request\n");
    ret = mount_bdev(fs_type, flags, dev_name, data, assoofs_fill_super);
    // Control de errores a partir del valor de ret. En este caso se puede utilizar la macro IS_ERR: if (IS_ERR(ret)) ...
    return ret;
}

/*
 *  assoofs file system type
 */
static struct file_system_type assoofs_type = {
    .owner = THIS_MODULE,
    .name = "assoofs",
    .mount = assoofs_mount,
    .kill_sb = kill_block_super,
};

static int __init assoofs_init(void)
{
    int ret;
    printk(KERN_INFO "assoofs_init request\n");
    ret = register_filesystem(&assoofs_type);
    // Control de errores a partir del valor de ret
    return ret;
}

static void __exit assoofs_exit(void)
{
    int ret;
    printk(KERN_INFO "assoofs_exit request\n");
    ret = unregister_filesystem(&assoofs_type);
    // Control de errores a partir del valor de ret
}

module_init(assoofs_init);
module_exit(assoofs_exit);
