// === include ===
// system path directory
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

// nested folder
#include "include/common.h"
#include "include/functions.h"

// === defines ===
#define	DEF_CONSOLE_TITLE	"file read"


/* GLOBAL declaration */
/* */
/* */	const TYPE_BYTE	*var_types[] = {
/* */										"HEX",
/* */										"BOOL",
/* */										"SINT",
/* */										"USINT",
/* */										"INT",
/* */										"UINT",
/* */										"DINT",
/* */										"UDINT",
/* */										"LINT",
/* */										"ULINT",
/* */										"REAL",
/* */										"LREAL"
/* */	};
/* */
/* */	const size_t	var_types_qntt = sizeof(var_types) / sizeof (var_types[0]);
/* */
/* END declaration */


int main(void) {
/***************/
/* declaration */
/* */
/* */	TYPE_BOOL		flag_DATA_edit_disable = FALSE;
/* */	TYPE_BOOL		fct_result_BOOL;
/* */	TYPE_rtn_cell	*matrix_rtn_src;
/* */	size_t			matrix_rtn_idx = 0;
/* */	size_t			matrix_rtn_ofs = 0;
/* */
/* */	TYPE_UINT		tmp_var = 0x2211;
/* */
/* */	size_t			idx = 0;
/* */
/* END declaration */
/*******************/

// === code ===
fct_con_set_size(DEF_CON_COLS, DEF_CON_LINES);
SetConsoleTitleA(DEF_CONSOLE_TITLE);

//	checking free space
if (fct_get_disk_free_space() != 1) {
	flag_DATA_edit_disable = TRUE;
	printf("WARNING: editing DATA file disabled\n");
}

//	allocating heap memory for source data
matrix_rtn_src = calloc(DEF_rtn_qntt, sizeof(TYPE_rtn_cell));
if (matrix_rtn_src == NULL) {
	fprintf(stderr, "ERROR: memory allocation for source failed\n");
	fct_exit(1);
}

matrix_rtn_idx = 0;
matrix_rtn_ofs = 0;
fct_result_BOOL = fct_array_set_generic(&(array_set_generic_params){
	.arr_src        = matrix_rtn_src,
	.arr_size_bytes = DEF_rtn_qntt * sizeof(TYPE_rtn_cell),
	.byte_offset    = matrix_rtn_idx * sizeof(TYPE_rtn_cell) + matrix_rtn_ofs,
	.set_value      = &tmp_var,
	.set_value_size = sizeof(tmp_var)
});
if (!fct_result_BOOL) {
	printf("error arr set\n");
}

for (idx = 0; idx < sizeof(TYPE_rtn_cell); idx = idx + 1) {
	printf("%u = %x\n", idx, matrix_rtn_src[0][idx]);
}

//	open and parse CSV
if (fct_load_and_parse_csv() < 0) {
	fct_exit(2);
}



//	heap memory free
free(matrix_rtn_src);
//	destroy pointer after use
matrix_rtn_src = NULL;


fct_exit(0);

return 0;
}	// main()

