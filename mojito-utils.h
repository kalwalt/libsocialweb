#include <glib.h>
#include <time.h>
#include <sqlite3.h>

/* TODO: these are not used at the moment */
time_t mojito_time_t_from_string (const char *s);
char * mojito_time_t_to_string (time_t t);
char * mojito_date_to_iso (const char *s);

gboolean mojito_create_tables (sqlite3 *db, const char *sql);
