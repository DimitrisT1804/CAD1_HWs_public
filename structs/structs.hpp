#ifndef _STRUCTS_HPP
#define _STRUCTS_HPP
#include "structs_def.hpp"

#define ENABLE_PRINTS
// #undef ENABLE_PRINTS

Pin *getPinFromFullName(Module *, string);
string getFullNameFromPin(Pin *);
Module *getModuleFromFullName(Module *, string);
Cell *getCellFromFullName(Module *, string);
Port *getPortFromFullName(Module *, string);

void unit_test_structs();
int create_data_structures();
void getLUTDataFromTimingArc(LibTimingArc *, LUTType, long double **, int *, long double **, int *, long double **);


extern "C" {
  // name and fullname here points to ASP internal structures. do not free!
  extern char ASP_get_next_module_name_and_hier_name(char **name, char **fullname);

  // fullname is dynamically reallocated on every call. Free it after done with
  // all calls.
  extern char ASP_get_next_cell_full_name_libcell_and_size(char **fullname,
                                                           char **libcellname,
                                                           float *h, float *w);

    // fullname is dynamically reallocated on every call. Free it after done
    // with all calls.
    extern char ASP_get_next_pin_info(char **fullname, float *x, float *y,
                                      float *h, float *w, char *direction);
    // fullname is dynamically reallocated on every call. Free it after done
    // with all calls.
    extern char ASP_get_next_port_info(char **fullname, float *x, float *y,
                                       float *h, float *w, char *direction);

    extern char ASP_get_next_net_info(char **fullname, char ***consnames,
                                      int *consnum);
  }
#endif
