#ifndef HISTORY_H
#define HISTORY_H

#define MAX_HISTORY 100

void history_init(void);

void history_add(const char *command);

void history_print(void);

#endif
