// === include ===
// system path directory
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <windows.h>

// current folder
#include "functions.h"
#include "common.h"

// === defines ===

//////////////////
void	fct_exit(const TYPE_INT IN_exit_code) {
	// === declaration ===
	TYPE_BYTE	pressed_key = 0;

	// === code ===
	printf("\nReturn code %d, press ESC to exit...\n", IN_exit_code);
	while (TRUE) {
		pressed_key = fct_get_pressed_key();
		if (pressed_key != 0) {
			if (pressed_key == DEF_KEY_ESC) {
				exit(IN_exit_code);
			}
		}
	}
}


//////////////////
TYPE_BYTE	fct_get_pressed_key(void) {
	// === declaration ===
	TYPE_INT	pressed_key;

	// === code ===
	pressed_key = _getch();
	if (pressed_key == DEF_KEYS_SPECIAL) {
		pressed_key = _getch();
		if ((pressed_key == DEF_KEY_ARROW_UP) || (pressed_key == DEF_KEY_ARROW_DOWN)) {
			return pressed_key;
		} else {
			return 0;
		}
	} else if ((pressed_key == DEF_KEY_ENTER) || (pressed_key == DEF_KEY_ESC)) {
		return pressed_key;
	} else {
		return 0;
	}
}


//////////////////
void	fct_con_set_size(const TYPE_UINT IN_con_cols, const TYPE_UINT IN_con_lines) {
	// === declaration ===
	HANDLE		handle_con;
	COORD		con_buffer_size;
	SMALL_RECT	con_window_size;

	// === code ===
	handle_con = GetStdHandle(STD_OUTPUT_HANDLE);
	if (handle_con == INVALID_HANDLE_VALUE) {
		perror("");
		return;
	}

	//	перед заданием размера окна задаем размер буфера не менее размера окна
	con_buffer_size.X = (SHORT)IN_con_cols;
	con_buffer_size.Y = (SHORT)IN_con_lines;
	SetConsoleScreenBufferSize(handle_con, con_buffer_size);

	//	задаем размер видимого окна (координаты от 0,0 до cols-1, lines-1)
	con_window_size.Left  = 0;
	con_window_size.Top   = 0;
	con_window_size.Right = (SHORT)(IN_con_cols - 1);
	con_window_size.Bottom = (SHORT)(IN_con_lines - 1);
	SetConsoleWindowInfo(handle_con, TRUE, &con_window_size);

	return;
}


//////////////////
TYPE_INT	fct_get_disk_free_space(void) {
	// === declaration ===
	ULARGE_INTEGER	struct_bytes_ttl, struct_bytes_ttl_free, struct_bytes_avail_free;

	// === code ===
	//	запрашиваем у ОС свободное место на диске
	if (GetDiskFreeSpaceExA(".", &struct_bytes_avail_free, &struct_bytes_ttl, &struct_bytes_ttl_free)) {
		//	если свободного места меньше - запоминаем
		if (struct_bytes_avail_free.QuadPart < 100ULL * 1024 * 1024) {
			printf("Disk space total: %llu MB\n", struct_bytes_ttl.QuadPart / (1024 * 1024));
			printf("Disk space free available: %llu MB\n", struct_bytes_avail_free.QuadPart / (1024 * 1024));
			printf("Disk space free total: %llu MB\n", struct_bytes_ttl_free.QuadPart / (1024 * 1024));
			printf("WARNING: not enought free space for editing DATA file\n");
			return 0;
		} else {
			return 1;
		}
	} else {
		fprintf(stderr, "ERROR: check free space failed (%lu)\n", GetLastError());
		return -1;
	}
}


//////////////////
TYPE_INT	fct_load_bin(TYPE_rtn_cell * const matrix_rtn_src) {
	// === declaration ===
	FILE		*pointer_file_BIN;
	size_t		rtn_cell_idx = 0;
	size_t		bytes_read = 0;
	TYPE_BYTE	tmp_buffer[DEF_rtn_cell_size] = {0};

	// === code ===
	//	trying to open CSV file
	pointer_file_BIN = fopen(DEF_file_name_BIN, "r");
	if (NULL == pointer_file_BIN) {
		switch (errno) {
			case 2:
				printf("WARNING: BIN file opening failed (error code %d - no such file)\n", errno);
				printf("WARNING: using default configuration (all values is 0)\n");
				return 0;

			default:
				fprintf(stderr, "ERROR: BIN file opening failed (error code %d) - ", errno);
				perror("");
				return -1;
		}
	}

	//	читаем по циклам, пока не упремся в конец файла или лимит строк
	while (rtn_cell_idx < DEF_rtn_qntt) {
		bytes_read = fread(tmp_buffer, sizeof(tmp_buffer[0]), sizeof(tmp_buffer), pointer_file_BIN);

		//	проверяем конец файла (EOF) или ошибку чтения
		if (bytes_read == 0) {
			if (feof(pointer_file_BIN)) {
				//	нормальный конец файла (EOF)
				break;
			} else if (ferror(pointer_file_BIN)) {
				fprintf(stderr, "ERROR: BIN file reading failed (error code %d) - ", errno);
				perror("");
				fclose(pointer_file_BIN);
				return -2;
			}
		}

		//	если это был неполный блок (файл не кратен DEF_rtn_cell_size байтам (обрезанный хвост))
		if (bytes_read < sizeof(tmp_buffer)) {
			//	значит файл закончился — выходим из цикла
			printf("WARNING: BIN file tail < cell size, ignoring last block\n");
			break;
		}

		// в элементы с 1 по размер буфера пишем содержимое буфера
		memcpy(&matrix_rtn_src[rtn_cell_idx][1], tmp_buffer, sizeof(tmp_buffer));

		rtn_cell_idx = rtn_cell_idx + 1;
	}
	printf("%u rtn cell(s) loaded from BIN file\n", rtn_cell_idx);

	fclose(pointer_file_BIN);
	return rtn_cell_idx;
}


//////////////////
TYPE_INT	fct_load_and_parse_csv(const load_and_parse_csv_params * const p_p) {
	// === declaration ===
	FILE		*pointer_file_CSV;
	TYPE_BYTE	tmp_arr_CSV_line[16] = {0};
	TYPE_BOOL	flag_CSV_new_line;
	size_t		tmp_CSV_line_len;
	size_t		tmp_CSV_line_num = 0;
	size_t		tmp_idx = 0;
	TYPE_BYTE	tmp_var_type = 0;

	// === code ===
	if (NULL == p_p) {
		return 0;	//	битый указатель на структуру
	}

	if (NULL == p_p -> matrix_rtn_src) {
		return 0;	//	битые указатели
	}

	//	trying to open CSV file
	pointer_file_CSV = fopen(DEF_file_name_CSV, "r");
	if (NULL == pointer_file_CSV) {
		switch (errno) {
			case 2:
				printf("WARNING: CSV file opening failed (error code %d - no such file)\n", errno);
				printf("WARNING: using default configuration (all values as HEX)\n");
				return 0;

			default:
				fprintf(stderr, "ERROR: CSV file opening failed (error code %d) - ", errno);
				perror("");
				return -1;
		}
	}

	while (fgets(tmp_arr_CSV_line, sizeof(tmp_arr_CSV_line), pointer_file_CSV) != NULL) {
		//	проверяем, поместилась ли строка в буфер целиком - если в конце буфера нет символа '\n'
		//	(и файл еще не кончился != NULL), значит строка была обрезана из-за превышения размера буфера
		tmp_CSV_line_len = strlen(tmp_arr_CSV_line);
		flag_CSV_new_line = tmp_CSV_line_len > 0 && '\n' == tmp_arr_CSV_line[tmp_CSV_line_len - 1];

		//	парсим буфер
		if (fct_parse_csv_line(&(parse_csv_line_params){
			.IN_CSV_line  = tmp_arr_CSV_line,
			.OUT_var_type = &tmp_var_type,
			.OUT_idx      = &tmp_idx
		})) {
			if (!fct_array_set_generic(&(array_set_generic_params){
				.arr_src        = p_p -> matrix_rtn_src,
				.arr_size_bytes = p_p -> matrix_rtn_size,
				.byte_offset    = p_p -> matrix_rtn_elem_size * tmp_idx + 0,
				.set_value      = &tmp_var_type,
				.set_value_size = sizeof(tmp_var_type)
			})) {
				printf("WARNING: arr set failed\n");
			}
		} else {
			printf("WARNING: skipping incorrect CSV line %u\n", tmp_CSV_line_num + 1);
		}

		//	строка НЕ влезла целиком, счищаем её остаток до конца файла или до '\n'
		if (!flag_CSV_new_line) {
			TYPE_INT	tmp_get_char;
			//	читаем символы по одному из файла и просто «выбрасываем» их, пока не встретим перенос строки или конец файла
			do {
				tmp_get_char = fgetc(pointer_file_CSV);
			} while (tmp_get_char != '\n' && tmp_get_char != EOF);
		}

		tmp_CSV_line_num = tmp_CSV_line_num + 1;
	}

	fclose(pointer_file_CSV);
	return 1;
}


//////////////////
TYPE_BYTE	fct_parse_csv_line(const parse_csv_line_params * const p_p) {
	// === declaration ===
	const char	*pointer_semicolon_1st;
	const char	*pointer_semicolon_2nd;
	char		*pointer_convert_end;
	TYPE_UDINT	tmp_idx;
	size_t		tmp_var_type_len;
	TYPE_BYTE	tmp_arr_var_type[8];
	TYPE_BOOL	type_found = 0;
	size_t		idx;

	// === code ===
	if (NULL == p_p) {
		return 0;	//	битый указатель на структуру
	}

	if (NULL == p_p -> IN_CSV_line || NULL == p_p -> OUT_var_type || NULL == p_p -> OUT_idx) {
		return 0;	//	битые указатели
	}

	//	ищем первый разделитель ';' в строке
	pointer_semicolon_1st = strchr(p_p -> IN_CSV_line, ';');
	if (NULL == pointer_semicolon_1st) {
		return 0;	//	нет разделителя
	}

	//	конвертируем всё, что до первого ';' в индекс в десятичной системе счисления
	tmp_idx = strtoul(p_p -> IN_CSV_line, &pointer_convert_end, 10);
	//	между концом преобразования и ';' был мусор
	if (pointer_convert_end != pointer_semicolon_1st) {
		return 0;
	}
	//	индекс больше количества rtn в файле
	if (tmp_idx >= DEF_rtn_qntt) {
		return 0;
	}

	//	ищем второй разделитель ';' в строке
	pointer_semicolon_2nd = strchr(pointer_semicolon_1st + 1, ';');
	if (NULL == pointer_semicolon_2nd) {
		return 0;	//	нет разделителя
	}

	//	вычисляем длину текста между разделителями
	tmp_var_type_len = pointer_semicolon_2nd - (pointer_semicolon_1st + 1);
	if (tmp_var_type_len >= sizeof(tmp_arr_var_type)) {
		return 0;	//	текст слишком длинный для нашего буфера
	}

	//	копируем тип в буфер
	memcpy(tmp_arr_var_type, pointer_semicolon_1st + 1, tmp_var_type_len);
	//	ставим ноль-терминатор строки
	tmp_arr_var_type[tmp_var_type_len] = '\0';

	//	проходим циклом по всему массиву констант
	for (idx = 0; idx < var_types_qntt; idx = idx + 1) {
		//	сравниваем то, что получили из файла, с элементом массива
		if (0 == strcmp(tmp_arr_var_type, var_types[idx])) {
			//	нашли совпадение
			type_found = 1;
			//	дальше искать нет смысла, выходим из цикла
			break;
		}
	}

	//	тип не найден в списке разрешенных констант
	if (!type_found) {
		return 0;
	}

	*(p_p -> OUT_var_type) = idx;
	*(p_p -> OUT_idx) = (size_t)tmp_idx;

	return 1;
}


//////////////////
TYPE_BYTE	fct_array_set_generic(const array_set_generic_params * const p_p) {
	// === code ===
	if (NULL == p_p) {
		return 0;	//	битый указатель на структуру
	}

	if (NULL == p_p -> arr_src || NULL == p_p -> set_value) {
		return 0;	//	битые указатели на элементы структуры
	}

	//	проверяем, что смещение + размер данных НЕ выходят за рамки всего буфера в байтах!
	if (p_p -> byte_offset + p_p -> set_value_size > p_p -> arr_size_bytes) {
		return 0;	//	выход за границы массива
	}

	//	копируем байты значения по нужному адресу
	memcpy((TYPE_BYTE *)p_p -> arr_src + p_p -> byte_offset, p_p -> set_value, p_p -> set_value_size);

	return 1;
}


//////////////////
void	fct_print_table(TYPE_rtn_cell * const matrix_rtn_src, const size_t rtn_cells_loaded) {
	// === declaration ===
	size_t			idx = 0;
	TYPE_SINT		tmp_SINT = 0;
	TYPE_USINT		tmp_USINT = 0;
	TYPE_INT		tmp_INT = 0;
	TYPE_UINT		tmp_UINT = 0;
	TYPE_DINT		tmp_DINT = 0;
	TYPE_UDINT		tmp_UDINT = 0;
	TYPE_LINT		tmp_LINT = 0;
	TYPE_ULINT		tmp_ULINT = 0;

	// === code ===
	if (rtn_cells_loaded > 0) {
		printf("\nIDX\tTYPE\tVALUE\n");

	//	for (idx = 0; idx < DEF_rtn_qntt; idx = idx + 1) {
		for (idx = 0; idx < rtn_cells_loaded; idx = idx + 1) {
			printf("[%4u]\t", idx);
			printf("%5s\t", var_types[matrix_rtn_src[idx][0]]);

			switch (matrix_rtn_src[idx][0]) {
				case 0:
					fct_print_HEX(&matrix_rtn_src[idx][1], sizeof(matrix_rtn_src[0]) - 1);
					putchar('\n');
					break;
				case 1:
					fct_print_BOOL(&matrix_rtn_src[idx][1], sizeof(matrix_rtn_src[0]) - 1);
					putchar('\n');
					break;
				case 2:
					memcpy(&tmp_SINT, &matrix_rtn_src[idx][1], sizeof(tmp_SINT));
					printf("%26d\n", tmp_SINT);
					break;
				case 3:
					memcpy(&tmp_USINT, &matrix_rtn_src[idx][1], sizeof(tmp_USINT));
					printf("%26u\n", tmp_USINT);
					break;
				case 4:
					memcpy(&tmp_INT, &matrix_rtn_src[idx][1], sizeof(tmp_INT));
					printf("%26d\n", tmp_INT);
					break;
				case 5:
					memcpy(&tmp_UINT, &matrix_rtn_src[idx][1], sizeof(tmp_UINT));
					printf("%26u\n", tmp_UINT);
					break;
				case 6:
					memcpy(&tmp_DINT, &matrix_rtn_src[idx][1], sizeof(tmp_DINT));
					printf("%26d\n", tmp_DINT);
					break;
				case 7:
					memcpy(&tmp_UDINT, &matrix_rtn_src[idx][1], sizeof(tmp_UDINT));
					printf("%26u\n", tmp_UDINT);
					break;
				case 8:
					memcpy(&tmp_LINT, &matrix_rtn_src[idx][1], sizeof(tmp_LINT));
					printf("%26lld\n", tmp_LINT);
					break;
				case 9:
					memcpy(&tmp_ULINT, &matrix_rtn_src[idx][1], sizeof(tmp_ULINT));
					printf("%26llu\n", tmp_ULINT);
					break;
			}
		}
	}

	return;
}


//////////////////
TYPE_BYTE	fct_print_BOOL(const void * const pointer_value, const size_t IN_value_size) {
	// === declaration ===
	size_t			idx = 0;
	TYPE_LWORD		tmp_val = 0;

	// === code ===
	if (NULL == pointer_value) {
		return 0;	//	битый указатель
	}

	memcpy(&tmp_val, pointer_value, (IN_value_size > sizeof(tmp_val)) ? sizeof(tmp_val) : IN_value_size);

	//	size_t is allways > 0, so goind down from 8 * sizeof(tmp_val) while get overflow
	for (idx = 8 * sizeof(tmp_val) - 1; idx < 8 * sizeof(tmp_val); idx = idx - 1) {
		putchar(((tmp_val >> idx) & 0x0001) ? '1' : '0');
		if (0 == (8 * sizeof(tmp_val) - idx) % 4) {
			putchar(' ');
		}
		if (0 == (8 * sizeof(tmp_val) - idx) % 8) {
			putchar(' ');
		}
	}

	return 1;
}


//////////////////
TYPE_BYTE	fct_print_HEX(const void * const pointer_value, const size_t IN_value_size) {
	// === declaration ===
	size_t			idx = 0, jdx = 0;
	TYPE_LWORD		tmp_val = 0;

	// === code ===
	if (NULL == pointer_value) {
		return 0;	//	битый указатель
	}

	memcpy(&tmp_val, pointer_value, (IN_value_size > sizeof(tmp_val)) ? sizeof(tmp_val) : IN_value_size);

	//	size_t is allways > 0, so goind down from sizeof(tmp_val) while get overflow
	for (idx = sizeof(tmp_val) - 1; idx < sizeof(tmp_val); idx = idx - 1) {
		for (jdx = 1; jdx < 2; jdx = jdx - 1) {
			switch ((((tmp_val >> 8 * idx) & 0x00FF) >> 4 * jdx) & 0x000F) {
				case 0x0:
					putchar('0');
					break;
				case 0x1:
					putchar('1');
					break;
				case 0x2:
					putchar('2');
					break;
				case 0x3:
					putchar('3');
					break;
				case 0x4:
					putchar('4');
					break;
				case 0x5:
					putchar('5');
					break;
				case 0x6:
					putchar('6');
					break;
				case 0x7:
					putchar('7');
					break;
				case 0x8:
					putchar('8');
					break;
				case 0x9:
					putchar('9');
					break;
				case 0xa:
					putchar('a');
					break;
				case 0xb:
					putchar('b');
					break;
				case 0xc:
					putchar('c');
					break;
				case 0xd:
					putchar('d');
					break;
				case 0xe:
					putchar('e');
					break;
				case 0xf:
					putchar('f');
					break;
			}
		}
		if (0 == (sizeof(tmp_val) - idx) % 1) {
			putchar(' ');
		}
		if (0 == (sizeof(tmp_val) - idx) % 2) {
			putchar(' ');
		}
	}

	return 1;
}



