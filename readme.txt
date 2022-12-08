This is a solution to day 1 of advent of code 2022 in SQL and C.

Used database: SQLite.

Strategy: import the input file "input" to SQLite table and run a SQL statement in the DB to get the desired result. DB is not created as a file, but kept in memory, SQLite allows that.

To run, install SQLite development dependency, a C compiler and run:

	sh run.sh
