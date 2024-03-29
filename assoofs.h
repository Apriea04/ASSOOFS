#define ASSOOFS_MAGIC 0x20200406
#define ASSOOFS_DEFAULT_BLOCK_SIZE 4096
#define ASSOOFS_FILENAME_MAXLEN 255
#define ASSOOFS_LAST_RESERVED_BLOCK ASSOOFS_ROOTDIR_BLOCK_NUMBER
#define ASSOOFS_LAST_RESERVED_INODE ASSOOFS_ROOTDIR_INODE_NUMBER
const int ASSOOFS_SUPERBLOCK_BLOCK_NUMBER = 0;
const int ASSOOFS_INODESTORE_BLOCK_NUMBER = 1;
const int ASSOOFS_ROOTDIR_BLOCK_NUMBER = 2;
const int ASSOOFS_ROOTDIR_INODE_NUMBER = 1;
const int ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED = 64;

// Extra: definimos las flags
#define ASSOOFS_FLAG_FREE 0
#define ASSOOFS_FLAG_USED 1

struct assoofs_super_block_info
{
    uint64_t version;
    uint64_t magic;
    uint64_t block_size;
    uint64_t inodes_count; // Extra: con el borrado, llevará la cuenta de los inodos reales del sistema
    uint64_t free_blocks;

    char padding[4056];
};

struct assoofs_dir_record_entry
{
    char filename[ASSOOFS_FILENAME_MAXLEN];
    uint64_t inode_no;
    uint64_t state_flag; // Controla si el dentry está borrado o usándose
};

struct assoofs_inode_info
{
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_number;

    union
    {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
    uint64_t state_flag; // Controla si el inodo está borrado o usándose
};
