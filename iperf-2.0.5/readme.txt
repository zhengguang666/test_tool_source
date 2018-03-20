./configure --host=arm-linux-gnueabihf
vim config.h
#define HAVE_MALLOC 0           改为    #define HAVE_MALLOC 1
#define malloc rpl_malloc       改为    /* #undef malloc */
make
