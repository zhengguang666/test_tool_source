typedef struct {
     char *name;                /* menu item name */
     void (*menuFunc)(int fd);  /* menu function */
} InxMenuItem_tt;

