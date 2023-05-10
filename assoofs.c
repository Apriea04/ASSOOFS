#include <linux/module.h>      /* Needed by all modules */
#include <linux/kernel.h>      /* Needed for KERN_INFO  */
#include <linux/init.h>        /* Needed for the macros */
#include <linux/fs.h>          /* libfs stuff           */
#include <linux/buffer_head.h> /* buffer_head           */
#include <linux/slab.h>        /* kmem_cache            */
#include "assoofs.h"

// TODO añadir trazas por método con "request"

MODULE_LICENSE("GPL");

void assoofs_save_sb_info(struct super_block *vsb)
{
    struct buffer_head *bh;
    struct assoofs_super_block_info *sb = vsb->s_fs_info; // Información persistente del superbloque en memoria
    bh = sb_bread(vsb, ASSOOFS_SUPERBLOCK_BLOCK_NUMBER);
    bh->b_data = (char *)sb; // Sobreescribimos los datos de disco con la información en memoria

    // Para que el cambio pase a disco, marcamos como sucio y sincronizamos:
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
}

/**
 * @brief Gives a free block to the given inode and updates the superblock info
 *
 */
int assoofs_sb_get_a_freeblock(struct super_block *sb, uint64_t *block)
{
    int i;
    // Información persistente del superbloque (campo s_fs_info)
    struct assoofs_super_block_info *assoofs_sb = sb->s_fs_info;

    // Recorremos en busca de un bloque libre:
    for (i = 2; i < ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED; i++)
    {
        if (assoofs_sb->free_blocks & (1 << i))
        {
            break;
            // Cuando aparece el primer bit 1 en free_block paramos, porque i tendrá la posición del primer bloque libre
        }
    }
    *block = i; // Guardamos el valor del bloque que se ocupará

    // Actualizamos free_blocks
    assoofs_sb->free_blocks &= ~(1 << i);
    assoofs_save_sb_info(sb);
    return 0; // Todo ha ido bien;
}

void assoofs_add_inode_info(struct super_block *sb, struct assoofs_inode_info *inode)
{
    struct assoofs_super_block_info *assoofs_sb;
    struct assoofs_inode_info *inode_info;
    struct buffer_head *bh;

    // Acceder a la información persistente del superbloque para obtener el contador de inodos:
    assoofs_sb = sb->s_fs_info;

    // Leer de disco el bloque que contiene el almacén de inodos:
    bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER);

    // Obtener un puntero al final del almacén y escribir un nuevo valor al final
    inode_info = (struct assoofs_inode_info *)bh->b_data;
    inode_info += assoofs_sb->inodes_count;
    memcpy(inode_info, inode, sizeof(struct assoofs_inode_info));

    // Marcar el bloque como sucio y sincronizar
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);

    // Actualizar el contador de inodos de la información persistente del superbloque y guardar los cambios:
    assoofs_sb->inodes_count++;
    assoofs_save_sb_info(sb);
}

struct assoofs_inode_info *assoofs_search_inode_info(struct super_block *sb, struct assoofs_inode_info *start, struct assoofs_inode_info *search)
{
    uint64_t count = 0;
    /*Recorremos el almacén de inodos desde start (principio del almacén)
    hasta encontrar los datos del inodo "search" o hasta el final del almacén
    */
    while (start->inode_no != search->inode_no && count < ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count)
    {
        count++;
        start++;
    }

    if (start->inode_no == search->inode_no)
    {
        return start;
    }
    else
    {
        return NULL;
    }
}

int assoofs_save_inode_info(struct super_block *sb, struct assoofs_inode_info *inode_info)
{
    struct buffer_head *bh;
    struct assoofs_inode_info *inode_pos;

    // Obtener de disco el almacén de inodos
    bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER);

    // Buscar los datos de inode_info en el almacén con una función auxiliar:
    inode_pos = assoofs_search_inode_info(sb, (struct assoofs_inode_info *)bh->b_data, inode_info);

    // Actualizar el inodo, marcar el bloque como sucio y sincronizar
    memcpy(inode_pos, inode_info, sizeof(*inode_pos));
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    return 0;
}

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
    sb = parent_inode->i_sb;
    bh = sb_bread(sb, parent_info->data_block_number);

    // Recorrer el contenido del directorio buscando la entrada cuyo nombre se corresponda con el que buscamos.
    // Cuando se localiza la entrada, se contruye el inodo correspondiente.
    record = (struct assoofs_dir_record_entry *)bh->b_data;
    for (i = 0; i < parent_info->dir_children_count; i++)
    {
        if (!strcmp(record->filename, child_dentry->d_name.name))
        {
            struct inode *inode = assoofs_get_inode(sb, record->inode_no);
            inode_init_owner(sb->s_user_ns, inode, parent_inode, ((struct assoofs_inode_info *)inode->i_private)->mode);
            d_add(child_dentry, inode);
            return NULL;
        }
        record++;
    }

    printk(KERN_INFO "No inode found\n");

    return NULL;
}

static int assoofs_create(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    struct inode *inode;
    struct super_block *sb;
    uint64_t count;
    struct assoofs_inode_info *inode_info;

    struct assoofs_inode_info *parent_inode_info;
    struct assoofs_dir_record_entry *dir_contents;

    struct buffer_head *bh;

    printk(KERN_INFO "New file request\n");

    sb = dir->i_sb;                                                           // puntero al superbloque desde dir
    count = ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count; // número de inodos de la información persistente del superbloque
    inode = new_inode(sb);
    inode->i_sb = sb;
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
    inode->i_op = &assoofs_inode_ops;
    inode->i_ino = ++count; // Asignar nuevo número al inodo a partir de count
    if (count > ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED)
    {
        printk(KERN_ERR "Max filesystem objects created\n");
    }
    else
    {
        printk(KERN_INFO "Filesystem objects less/equal than maximum\n");
    }

    inode_info = kmalloc(sizeof(struct assoofs_inode_info), GFP_KERNEL);
    inode_info->inode_no = inode->i_ino;
    inode_info->mode = mode; // El mode es un argumento;
    inode_info->file_size = 0;
    inode->i_private = inode_info;

    inode->i_fop = &assoofs_file_operations;

    // Propietarios y permisos
    inode_init_owner(sb->s_user_ns, inode, dir, mode);
    d_add(dentry, inode);

    // Obtenemos un nuevo bloque para el inodo:
    assoofs_sb_get_a_freeblock(sb, &inode_info->data_block_number);

    // Guardamos la información persistente
    assoofs_add_inode_info(sb, inode_info);

    // PASO 2: modificar el contenido del directorio padre añadiendo una nueva entrada para elnuevo archivo:

    parent_inode_info = dir->i_private;
    bh = sb_bread(sb, parent_inode_info->data_block_number);

    dir_contents = (struct assoofs_dir_record_entry *)bh->b_data;
    dir_contents += parent_inode_info->dir_children_count;
    dir_contents->inode_no = inode_info->inode_no; // inode_info es la información persistente creada antes

    strcpy(dir_contents->filename, dentry->d_name.name);
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);

    // PASO 3: actualizar la información persistente del inodo padre:
    // ahora tiene un archivo más

    parent_inode_info->dir_children_count++;
    assoofs_save_inode_info(sb, parent_inode_info);
    return 0;
}

static int assoofs_mkdir(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    // TODO unificar el código con assoofs_create con ifs/elses para las diferencias:

    struct inode *inode;
    struct super_block *sb;
    uint64_t count;
    struct assoofs_inode_info *inode_info;

    struct assoofs_inode_info *parent_inode_info;
    struct assoofs_dir_record_entry *dir_contents;

    struct buffer_head *bh;

    printk(KERN_INFO "New directory request\n");

    sb = dir->i_sb;                                                           // puntero al superbloque desde dir
    count = ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count; // número de inodos de la información persistente del superbloque
    inode = new_inode(sb);
    inode->i_sb = sb;
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
    inode->i_op = &assoofs_inode_ops;
    inode->i_ino = ++count; // Asignar nuevo número al inodo a partir de count
    if (count > ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED)
    {
        printk(KERN_ERR "Max filesystem objects created\n");
    }
    else
    {
        printk(KERN_INFO "Filesystem objects less/equal than maximum\n");
    }

    inode_info = kmalloc(sizeof(struct assoofs_inode_info), GFP_KERNEL);
    inode_info->inode_no = inode->i_ino;
    inode_info->mode = S_IFDIR | mode; // El mode es un argumento;
    inode_info->dir_children_count = 0;
    inode->i_private = inode_info;

    inode->i_fop = &assoofs_dir_operations;

    // Propietarios y permisos
    inode_init_owner(sb->s_user_ns, inode, dir, inode_info->mode);
    d_add(dentry, inode);

    // Obtenemos un nuevo bloque para el inodo:
    assoofs_sb_get_a_freeblock(sb, &inode_info->data_block_number);

    // Guardamos la información persistente
    assoofs_add_inode_info(sb, inode_info);

    // PASO 2: modificar el contenido del directorio padre añadiendo una nueva entrada para elnuevo archivo:

    parent_inode_info = dir->i_private;
    bh = sb_bread(sb, parent_inode_info->data_block_number);

    dir_contents = (struct assoofs_dir_record_entry *)bh->b_data;
    dir_contents += parent_inode_info->dir_children_count;
    dir_contents->inode_no = inode_info->inode_no; // inode_info es la información persistente creada antes

    strcpy(dir_contents->filename, dentry->d_name.name);
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);

    // PASO 3: actualizar la información persistente del inodo padre:
    // ahora tiene un archivo más

    parent_inode_info->dir_children_count++;
    assoofs_save_inode_info(sb, parent_inode_info);
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
    sb->s_fs_info = assoofs_sb;
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
