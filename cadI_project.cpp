//   ____    _    ____    ___ ___    ____                            ____            _           _
//  / ___|  / \  |  _ \  |_ _|_ _|  / ___|___  _   _ _ __ ___  ___  |  _ \ _ __ ___ (_) ___  ___| |_
// | |     / _ \ | | | |  | | | |  | |   / _ \| | | | '__/ __|/ _ \ | |_) | '__/ _ \| |/ _ \/ __| __|
// | |___ / ___ \| |_| |  | | | |  | |__| (_) | |_| | |  \__ \  __/ |  __/| | | (_) | |  __/ (__| |_
//  \____/_/   \_\____/  |___|___|  \____\___/ \__,_|_|  |___/\___| |_|   |_|  \___// |\___|\___|\__|
//                                                                                |__/
//
// A project by University of Thessaly CASlab and Stavros Simoglou, for the
// purpose of academic training on the topics of physical design algorithms, EDA
// CAD, and larger project development
//
// We hope this project will spark your interest in the above topics and that
// you will enjoy the assignments.
//
// Happy Hacking!
//
// File:    cadI_project.cpp
// Purpose: main CAD I project entry point
// Authors: Stavros Simoglou
//
#include <iostream>
#include <structs.hpp>
#include <tcl.h>
#include <string.h>
#include <tclDecls.h>
#include "cadI_project.hpp"


int load_liberty(ClientData clientdata, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv) {
  char *libfilename = NULL;

  if (objc != 2) {
    Tcl_SetResult(interp, (char *)"Usage: load_liberty <libfile>", TCL_STATIC);
    return TCL_ERROR;
  }

  libfilename = Tcl_GetString((Tcl_Obj *) objv[1]);

  int ret = parse_liberty(libfilename, &mainLibrary);
  if (mainLibrary == NULL) {
    Tcl_SetResult(interp, (char *)"Error parsing liberty file", TCL_STATIC);
    return TCL_ERROR;
  }
  mainLibrary->print();
  mainLibrary->write_lef_for_ASP("/tmp/asp.lef");
  tclwrapeval((char *)"load_lef /tmp/asp.lef");
  return ret;
}

int write_verilog(ClientData clientdata, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv) {
  char *outputfile = NULL;
  string filename;

  if (objc != 2) {
    Tcl_SetResult(interp, (char *)"Usage: write_verilog <output file>", TCL_STATIC);
    return TCL_ERROR;
  }

  outputfile = Tcl_GetString((Tcl_Obj *) objv[1]);
  filename = string(outputfile);

  if (topModule == NULL) {
    Tcl_SetResult(interp, (char *)"Error. Top module is not defined.", TCL_STATIC);
    return TCL_ERROR;
  }

  topModule->write_netlist(filename);

  return TCL_OK;
}

int run_unit_test(ClientData clientdata, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv) {

  unit_test_structs();

  return TCL_OK;
}

int main(int argc, char *argv[]) {

  std::cout << "Init TCL" << std::endl;

  // allocate tcl command names
  cadII_tcl_commands_num = 4;
  cmdNames    = (char **) calloc(cadII_tcl_commands_num, sizeof(char *));
  cmdNames[0] = strdup("create_data_structures");
  cmdNames[1] = strdup("load_liberty");
  cmdNames[2] = strdup("write_verilog");
  cmdNames[3] = strdup("run_unit_test");


  procs    = (Tcl_ObjCmdProc **)calloc(cadII_tcl_commands_num,
                                       sizeof(Tcl_ObjCmdProc *));
  procs[0] = (Tcl_ObjCmdProc *) create_data_structures;
  procs[1] = (Tcl_ObjCmdProc *) load_liberty;
  procs[2] = (Tcl_ObjCmdProc *) write_verilog;
  procs[3] = (Tcl_ObjCmdProc *) run_unit_test;


  start_console();
  
  return 1;
}