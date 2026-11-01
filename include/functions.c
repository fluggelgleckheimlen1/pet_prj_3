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
void	fct_exit(TYPE_INT exit_code) {
	// === declaration ===
	TYPE_BYTE	pressed_key = 0;

	// === code ===
	printf("\nReturn code %d, press ESC to exit...\n", exit_code);
	while (TRUE) {
		pressed_key = fct_get_pressed_key();
		if (pressed_key != 0) {
			if (pressed_key == DEF_KEY_ESC) {
				exit(exit_code);
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
void	fct_con_set_size(const TYPE_UINT con_cols, const TYPE_UINT con_lines) {
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
	con_buffer_size.X = (SHORT)con_cols;
	con_buffer_size.Y = (SHORT)con_lines;
	SetConsoleScreenBufferSize(handle_con, con_buffer_size);

	//	задаем размер видимого окна (координаты от 0,0 до cols-1, lines-1)
	con_window_size.Left  = 0;
	con_window_size.Top   = 0;
	con_window_size.Right = (SHORT)(con_cols - 1);
	con_window_size.Bottom = (SHORT)(con_lines - 1);
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
TYPE_INT	fct_load_and_parse_csv(void) {
	// === declaration ===
	FILE		*pointer_file_CSV;
	TYPE_BYTE	tmp_arr_CSV_line[16] = {0};
	TYPE_BOOL	flag_CSV_new_line;
	size_t		tmp_CSV_line_len;
	size_t		tmp_CSV_line_num = 0;
	size_t		tmp_idx = 0;
	TYPE_BYTE	tmp_var_type = 0;

	// === code ===
	//	trying to open CSV file
	pointer_file_CSV = fopen(DEF_file_name_CSV, "r");
	if (pointer_file_CSV == NULL) {
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

	printf("INFO: opening and parsing CSV...\n");

	while (fgets(tmp_arr_CSV_line, sizeof(tmp_arr_CSV_line), pointer_file_CSV) != NULL) {
		//	проверяем, поместилась ли строка в буфер целиком - если в конце буфера нет символа '\n'
		//	(и файл еще не кончился != NULL), значит строка была обрезана из-за превышения размера буфера
		tmp_CSV_line_len = strlen(tmp_arr_CSV_line);
		flag_CSV_new_line = tmp_CSV_line_len > 0 && tmp_arr_CSV_line[tmp_CSV_line_len - 1] == '\n';

		//	парсим буфер
		if (fct_parse_csv_line(&(parse_csv_line_params){
			.IN_CSV_line  = tmp_arr_CSV_line,
			.OUT_var_type = &tmp_var_type,
			.OUT_idx      = &tmp_idx
		})) {
			printf("[%04u]\t%u\n", tmp_idx, tmp_var_type);
		} else {
			printf("WARNING: skipping incorrect line %u\n", tmp_CSV_line_num);
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
TYPE_UINT	fct_parse_csv_line(const parse_csv_line_params * const p_p) {
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
	if (p_p == NULL ) {
		return 0;	//	битый указатель на структуру
	}

	if (p_p -> IN_CSV_line == NULL || p_p -> OUT_var_type == NULL || p_p -> OUT_idx == NULL) {
		return 0;	//	битые указатели
	}

	//	ищем первый разделитель ';' в строке
	pointer_semicolon_1st = strchr(p_p -> IN_CSV_line, ';');
	if (pointer_semicolon_1st == NULL) {
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
	if (pointer_semicolon_2nd == NULL) {
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
		if (strcmp(tmp_arr_var_type, var_types[idx]) == 0) {
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
	if (p_p == NULL ) {
		return 0;	//	битый указатель на структуру
	}

	if (p_p -> arr_src == NULL || p_p -> set_value == NULL ) {
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

