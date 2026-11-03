#if !defined FUNCTIONS_H
#define FUNCTIONS_H

// === include ===
// current folder
#include "common.h"

// === defines ===

// === declaration ===
void		fct_exit(const TYPE_INT);
TYPE_BYTE	fct_get_pressed_key(void);
void		fct_con_set_size(const TYPE_UINT, const TYPE_UINT);

TYPE_INT	fct_get_disk_free_space(void);

TYPE_INT	fct_load_bin(TYPE_rtn_cell * const);

typedef struct {
	TYPE_rtn_cell * const	matrix_rtn_src;
	const size_t			matrix_rtn_size;
	const size_t			matrix_rtn_elem_size;
} load_and_parse_csv_params;
TYPE_INT	fct_load_and_parse_csv(const load_and_parse_csv_params * const);


typedef struct {
	const TYPE_BYTE * const	IN_CSV_line;
	TYPE_BYTE * const		OUT_var_type;
	size_t * const			OUT_idx;
} parse_csv_line_params;
TYPE_BYTE	fct_parse_csv_line(const parse_csv_line_params * const);


typedef struct {
	void * const		arr_src;
	const size_t		arr_size_bytes;
	const size_t		byte_offset;
	const void * const	set_value;
	const size_t		set_value_size;
} array_set_generic_params;
TYPE_BYTE	fct_array_set_generic(const array_set_generic_params * const);

void		fct_print_table(TYPE_rtn_cell * const, const size_t);
TYPE_BYTE	fct_print_BOOL(const void * const, const size_t);
TYPE_BYTE	fct_print_HEX(const void * const, const size_t);

#endif
