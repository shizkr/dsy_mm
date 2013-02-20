#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_PRINT		0x0000
#define DEBUG_SEARCH	0x0001
#define DEBUG_MAP		0x0002
#define DEBUG_CONTOUR	0x0004
#define DEBUG_BINTREE	0x0008
#define DEBUG_S_LINK	0x0010
#define DEBUG_DIAGNODE	0x0020
#define DEBUG_VORBOSE	0x8000

#define DEBUG_LEVEL \
	(DEBUG_SEARCH | DEBUG_MAP | DEBUG_CONTOUR | \
	 DEBUG_BINTREE | DEBUG_S_LINK)

#ifndef TAG
#define TAG
#endif

#ifdef DEBUG
#define print_dbg(flag, fmt, ...) \
({ \
	if (!flag || (flag & DEBUG_VORBOSE) || \
		(flag & debug_flag)) \
		printf(TAG fmt, ##__VA_ARGS__); \
})

#else
#define print_dbg(flag, fmt, ...) { }
#endif

#define print_info(fmt, ...) \
	printf(TAG fmt, ##__VA_ARGS__);
#define print_error(fmt, ...) \
	printf(TAG "ERROR:" fmt, ##__VA_ARGS__);

#define print_exit(fmt, ...) \
({ \
	printf(TAG fmt, ##__VA_ARGS__); \
	exit(EXIT_FAILURE); \
})

#endif
