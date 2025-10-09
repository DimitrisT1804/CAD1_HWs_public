#ifndef _CADI_PROJECT_HPP
#define _CADI_PROJECT_HPP

#include <tcl.h>
#include <structs_def.hpp>
#include <lib.hpp>

Module *topModule;
Lib *mainLibrary;

// don't touch! libasp extern definitions
extern "C" {

extern char **cmdNames;
extern Tcl_ObjCmdProc **procs;
extern int cadII_tcl_commands_num;

extern void start_console();
extern Tcl_Interp *tclinterp; // TCL Interpreter global variable //
extern void tclwrapeval(char *command);
}

extern int parse_liberty(string libfile_path, Lib **library);

#endif
