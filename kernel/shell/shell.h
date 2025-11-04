#ifndef SHELL_H
#define SHELL_H

void shell_clear(int argc, char** argv);
void shell_echo(int argc, char** argv);
void shell_exit(int argc, char** argv);
void shell_mkfs(int argc, char** argv);
void shell_ls(int argc, char** argv);
void shell_stat(int argc, char** argv);

void shell_main(void);

#endif
