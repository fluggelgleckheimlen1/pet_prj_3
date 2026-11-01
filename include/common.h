#if !defined COMMON_H
#define COMMON_H

// === include ===
// system path directory
#include <stdint.h>

// === defines ===
#define	DEF_CON_COLS		130U
#define	DEF_CON_LINES		32U

#define	DEF_KEYS_SPECIAL	0xe0U
#define	DEF_KEY_ARROW_UP	0x48U
#define	DEF_KEY_ARROW_DOWN	0x50U
#define	DEF_KEY_ENTER		0x0dU
#define	DEF_KEY_ESC			0x1bU

#define	DEF_file_name_CSV	"rtn_data.csv"
#define	DEF_rtn_qntt		8192U

// === declaration ===
/*              0..2e64 - 1      */	typedef uint64_t	TYPE_ULINT;
/*          -2e63..2e63 - 1      */	typedef int64_t		TYPE_LINT;
/*              0..4 294 967 295 */	typedef uint32_t	TYPE_UDINT;
/* -2 147 483 648..2 147 483 647 */	typedef int32_t		TYPE_DINT;
/*              0..       65 535 */	typedef uint16_t	TYPE_UINT;
/*        -32 768.........32 767 */	typedef int16_t		TYPE_INT;
/*              0............255 */	typedef uint8_t		TYPE_USINT;
/*           -128............127 */	typedef int8_t		TYPE_SINT;

typedef uint32_t	TYPE_DWORD;
typedef uint16_t	TYPE_WORD;
typedef uint8_t		TYPE_BYTE;
typedef uint8_t		TYPE_BOOL;

extern const TYPE_BYTE	*var_types[];
extern const size_t		var_types_qntt;
typedef uint8_t			TYPE_rtn_cell[9];

#endif
