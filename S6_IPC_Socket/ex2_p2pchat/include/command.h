// commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_help();
void cmd_connect(const char *ip, const char *portstr);
void cmd_list();
void cmd_terminate(int id);
void cmd_send(int id, const char *msg);
void cmd_exit();

#endif
