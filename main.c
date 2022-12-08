/*
Copyright (c) 2022 Alan Urmancheev <alan.urman@gmail.com>

Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
https://www.sqlite.org/quickstart.html
https://www.sqlite.org/cintro.html
https://www.sqlite.org/c3ref/stmt.html
https://www.sqlite.org/c3ref/bind_blob.html
https://www.sqlite.org/c3ref/prepare.html
https://www.sqlite.org/c3ref/step.html
*/

// Getline wrapper.
// Returns if line has been read into "line".
// The line doesn't contain the newline and should be freed.
static bool _my_read_line(FILE *stream, char **line) {
	*line = 0;
	bool success = getline(
		line,
		&(size_t){0}, // Allocated size ignored.
		stream
	) != -1;
	if (success) {
		if ((*line)[strlen(*line) - 1] == '\n')
			(*line)[strlen(*line) - 1] = '\0';
	}
	return success;
}

// _my_read_line wrapper. Returns read line or 0.
static char *my_read_line(FILE *stream) {
	char *result;
	bool success = _my_read_line(stream, &result);
	if (success)
		return result;
	free(result);
	return 0;
}

static void my_debug(int elf_id, int item_id, char *line) {
	printf(
		"elf_id\t"
		"%d\t\t"
		"item_id\t"
		"%d\t\t"
		"calories\t"
		"%s\t\t"
		"\n",
		elf_id,
		item_id,
		line
	);
}

static void my_sql_insert(sqlite3 *db, int elf_id, int item_id, char *line) {
	sqlite3_stmt *stmt;
	assert(sqlite3_prepare_v2(
		db,
		"insert into input (elf_id, calories, item_id) values (?, ?, ?)",
		-1,
		&stmt,
		0
	) == SQLITE_OK);
	sqlite3_bind_int(stmt, 1, elf_id);
	sqlite3_bind_text(stmt, 2, line, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, item_id);
	assert(sqlite3_step(stmt) == SQLITE_DONE);
	sqlite3_finalize(stmt);
}

static void my_sql_select_rows(sqlite3 *db, char *stmt_str) {
	sqlite3_stmt *stmt;
	if (sqlite3_prepare_v2(db, stmt_str, -1, &stmt, 0) != SQLITE_OK) {
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
		assert(false);
	}
	int n_columns = sqlite3_column_count(stmt);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		for (int i = 0; i < n_columns; i++) {
			printf("%s\n", sqlite3_column_name(stmt, i));
			printf("%s\n", sqlite3_column_text(stmt, i));
		}
	}
	sqlite3_finalize(stmt);
}


// elf_id, calories, item_id
static void my_process_line(sqlite3 *db, char *line) {
	static int item_id = 0;
	static int elf_id = 0;

	if (!*line) {
		elf_id++;
	} else {
		my_sql_insert(db, elf_id, item_id, line);
//		my_debug(elf_id, item_id, line);
		item_id++;
	}
}

static sqlite3 *my_db_open(void) {
	sqlite3 *result;
	assert(sqlite3_open_v2(
		"db",
		&result,
		SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY,
		0
	) == SQLITE_OK);
	return result;
}

static void my_db_do(sqlite3 *db, char *stmt) {
	sqlite3_exec(db, stmt, 0, 0, 0);
}

int main(void) {
	FILE *input = fopen("input", "rb");
	sqlite3 *db;
	assert(db = my_db_open());
	my_db_do(db, "create table input (elf_id integer, item_id integer, calories integer)");
//	for (int i = 0; i < 10; i++) {}
	while (1) {
		char *line = my_read_line(input);
		if (!line) break;
		my_process_line(db, line);
		free(line);
	}
	my_sql_select_rows(
		db,
		" select sum(s) from ("
		" select sum(calories) as s"
		" from input"
		" group by elf_id"
		" order by s desc"
		" limit 3"
		" )"

/*
		" select sum(calories) as s"
		" from input"
		" group by elf_id"
		" order by s desc"
		" limit 1"
*/
	);
	fclose(input);
}

#if 0
static int my_callback(
	void *_unused,
	int n_columns,
	char **row,
	char **column_names
) {
	for (int i = 0; i < n_columns; i++) {
		printf("%s\n", column_names[i]);
		printf("%s\n", row[i]);
	}
}

static void my_sqlite3_proc(void) {
	sqlite3 *c;
	do {
		if (sqlite3_open_v2(
			"db", // Ignored.
			&c,
				SQLITE_OPEN_READWRITE
			|
				SQLITE_OPEN_CREATE
			|
				SQLITE_OPEN_MEMORY
			,
			0 // Use default VFS.
		) != SQLITE_OK) {
			fprintf(stderr, "%s\n", sqlite3_errmsg(c));
			break;
		}
		sqlite3_exec(
			c,
			"create table input (elf_id integer, item_id integer, calories integer)",
			0, // Callback.
			0, // First arg to callback.
			0 // Error message.
		);
		sqlite3_exec(
			c,
			"select 2 + 2",
			my_callback,
			0, // First arg to callback.
			0 // Error message.
		);
	} while(false);
}
#endif
