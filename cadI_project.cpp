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
// Authors: Stavros Simoglou, Dimitris Tsalapatas
//
#include <iostream>
#include <structs.hpp>
#include <tcl.h>
#include <string.h>
#include <tclDecls.h>
#include <iostream>
#include <filesystem>
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

int get_cell_LUT(ClientData clientdata, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv) {

  if (objc != 3) {
    Tcl_SetResult(interp, (char *)"Usage: get_cell_LUT <cell_name> <lut_type: cell_rise, cell_fall, rise_transition, fall_transition>", TCL_STATIC);
    return TCL_ERROR;
  }

  char *cell_name = Tcl_GetString((Tcl_Obj *) objv[1]);
  char *lut_type_str = Tcl_GetString((Tcl_Obj *) objv[2]);

  LibCell *cell = mainLibrary->getCellByName(string(cell_name));
  if (!cell) {
    Tcl_SetResult(interp, (char *)"Error. Cell not found in library.", TCL_STATIC);
    return TCL_ERROR;
  }

  LUTType lut_type;
  if (strcmp(lut_type_str, "cell_rise") == 0) {
    lut_type = CELL_RISE;
  } 
  else if (strcmp(lut_type_str, "cell_fall") == 0){
    lut_type = CELL_FALL;
  } 
  else if (strcmp(lut_type_str, "rise_transition") == 0) {
    lut_type = RISE_TRANSITION;
  } 
  else if (strcmp(lut_type_str, "fall_transition") == 0) {
    lut_type = FALL_TRANSITION;
  } 
  else {
    Tcl_SetResult(interp, (char *)"Error. Unknown LUT type.", TCL_STATIC);
    return TCL_ERROR;
  }
   
  for (auto pin : cell->getPins()) {
    if (pin.second->getType() != OUTPUT) {
      continue;
    }
    cout << "->Pin: " << pin.second->getName() << endl;
    long double *input_slews = nullptr;
    long double *output_loads = nullptr;
    long double *values = nullptr;
    int slews_size = 0;
    int loads_size = 0;
    for (auto arc : pin.second->getTimingInfo()->out_tinfo->timing_arcs) {
      getLUTDataFromTimingArc(arc, lut_type, &input_slews, &slews_size,
                              &output_loads, &loads_size, &values);

      arc->print();
      cout << "slews: ";
      for (int i = 0; i < slews_size; i++) {
        cout << input_slews[i] << ", ";
      }
      cout << endl;
      cout << "loads: ";
      for (int i = 0; i < loads_size; i++) {
        cout << output_loads[i] << ", ";
      }
      cout << endl;
      cout << "values: ";
      for (int i = 0; i < loads_size*slews_size; i++) {
        cout << values[i] << ", ";
      }
      cout << endl;
    }
  }

  return TCL_OK;
}



int main(int argc, char *argv[]) {

  char *arguments = NULL;
  std::cout << "Init TCL" << std::endl;

  // allocate tcl command names
  cadII_tcl_commands_num = 5;
  cmdNames    = (char **) calloc(cadII_tcl_commands_num, sizeof(char *));
  cmdNames[0] = strdup("create_data_structures");
  cmdNames[1] = strdup("load_liberty");
  cmdNames[2] = strdup("write_verilog");
  cmdNames[3] = strdup("run_unit_test");
  cmdNames[4] = strdup("get_cell_LUT");


  procs    = (Tcl_ObjCmdProc **)calloc(cadII_tcl_commands_num,
                                       sizeof(Tcl_ObjCmdProc *));
  procs[0] = (Tcl_ObjCmdProc *) create_data_structures;
  procs[1] = (Tcl_ObjCmdProc *) load_liberty;
  procs[2] = (Tcl_ObjCmdProc *) write_verilog;
  procs[3] = (Tcl_ObjCmdProc *) run_unit_test;
  procs[4] = (Tcl_ObjCmdProc *) get_cell_LUT;


  if (argc > 1) {
    arguments = argv[1];
    if (!filesystem::exists(arguments)) {
      cout << "File " << arguments << " does not exist!." << endl;
      cout << "Starting interactive console." << endl;

      arguments = NULL;
    }
  }
  else {
    arguments = NULL;
  }
  start_console(arguments);

  cout << "Exiting CAD I Project." << endl;
  
  return 1;
}