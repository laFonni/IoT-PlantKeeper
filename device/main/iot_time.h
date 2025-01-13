#pragma once

extern bool time_synchronized;

void initialize_sntp(void);
void wait_for_time_sync(void);
void get_timestamp(char* timestamp);
