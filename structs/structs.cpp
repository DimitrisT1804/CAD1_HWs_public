
#include <structs_def.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <tcl.h>
#include "structs.hpp"
#include "lib.hpp"

#define SOLUTION

extern Module *topModule;
extern Lib *mainLibrary;


/* class Cell constructors */
// no physical information constructor
Cell::Cell(string name, string libcell_name, Instance *parent) {
  InstanceRef ref;
  ref.cellRef = this;
  setInstanceInfo(name, parent, Instance::CELL, ref);
  mLibcell = mainLibrary->getCellByName(libcell_name);
}

// physical information constructor
Cell::Cell(string name, string libcell_name, Instance *parent, float x, float y,
     float h, float w, OriginType o) {
  InstanceRef ref;
  ref.cellRef = this;
  setInstanceInfo(name, parent, Instance::CELL, ref);
  setCoordinates(&x, &y, &h, &w, &o);
  mLibcell = mainLibrary->getCellByName(libcell_name);
}

// destructor to clean pins map if allocated
Cell::~Cell() {
  // use ranged for to free all pins
  for (auto entry : mPins) {
    delete entry.second;
  }
}

// printer
void Cell::print() {
  cout << "Cell: " << endl;
  getInstance()->print();
  cout << "Type: " << mLibcell->getName() << endl;
  cout << "Pins: " << endl;
  for (auto entry : mPins) {
    entry.second->print();
  }
}

// write netlist method //
void Cell::write_netlist() {

  LibCell *libcell = getLibCell();
  if (libcell) {
    cout << libcell->getName() << " ";
  }
  else {
    cout << "unknown_libcell ";
  }

  cout << getName() << " ";

  // Start the list of connections 
  cout << "(";

  // Use a flag to handle the first element's formatting 
  bool firstPin = true;

  // Iterate through all the pins of the cell
  for (const auto& [pname, pptr] : sorted_items(mPins)) {
    // Add a comma and space before each subsequent pin
    if (!firstPin) {
      cout << ", ";
    }

    // Pin pointer 
    pptr->write_netlist();

    // After the first pin is printed, set the flag to false 
    firstPin = false;
  }

cout << ");" << endl;
}

// given a top module and a pin's full name, this function traverses hierarchy
// until reaching the pins and returns a pointer to the requested pin. Returns
// nullptr if search fails or input arguments are incorrect
Pin *getPinFromFullName(Module *top, string fullName) {

  // return nullptr if starting module is not specified
  if (!top) {
    return nullptr;
  }

  // tokenize full name on '/'
  vector<string> names;
  boost::split(names, fullName, boost::is_any_of("/"), boost::token_compress_off);

  // return nullptr if vector is smaller than 2. cannot happen as pin full name
  // looks like module1/module2/.../cell/pin
  if (names.size() < 2) {
    return nullptr;
  }

  // last name is pin name. use reference to avoid deep copy
  string &pinName = names[names.size() - 1];
  // second from last name is cell name. use reference to avoid deep copy
  string &cellName = names[names.size() - 2];

  size_t last = fullName.find_last_of('/');
  size_t second_from_end = fullName.find_last_of('/', last - 1);

  string module_path = fullName.substr(0, second_from_end);
  Module *curr = getModuleFromFullName(top, module_path);

  // find parent module
  if (!curr) {
    curr = top;
  }

  // now, curr contains last module. get cell
  Cell *cell = curr->getCell(cellName);

  // return nullptr if cell not found
  if (!cell) {
    return nullptr;
  }

  // get pin from cell
  Pin *pin = cell->getPin(pinName);

  // return nullptr if pin not found
  if (!pin) {
    return nullptr;
  }

  return pin;
}

// given a pin, this function returns the full hierarchical name of the pin as a
// string. pins are at the lowest level of hierarchy, so Instance hierarchy is
// traversed bottom-to-top using the parent pointers to create the full name
string getFullNameFromPin(Pin *pin) {
  // return empty string if pin is not specified
  if (!pin) {
    return "";
  }

  string fullName = pin->getName();

  // pin is bottom class in hierarchy. traverse hierarchy upwards until no
  // parent can be found and concat all instance names. don't add top module
  // name to full name as per EDA tools usual convention
  Instance *curr = pin->getParent();
  Instance *parent = nullptr;
  while (curr) {
    parent = curr->getParent();
    // if parent's parent is null, this is the top module, so break
    if (!parent) {
      break;
    }

    // concat already discovered name to current name. remember we are
    // traversing hierarchy from bottom to top
    fullName = curr->getName() + "/" + fullName;
    curr = parent;
  }

  return fullName;
}

// given a top module and a module's full name, this function traverses hierarchy
// until finding the module and returns a pointer to it. Returns
// nullptr if search fails or input arguments are incorrect
// NOTE: fullname should not contain top module name
Module *getModuleFromFullName(Module *top, string fullName) {

  // return nullptr if starting module is not specified
  if (!top) {
    return nullptr;
  }

  // tokenize full name on '/'
  vector<string> names;
  boost::split(names, fullName, boost::is_any_of("/"), boost::token_compress_off);

  // return top if vector size is 1. it means no slashes existed in fullName
  if ((names.size() == 1) && (names[0] == "")) {
    return top;
  }

  // last name in list is target module name
  string target = *(names.end() - 1);  //
  // start with top
  Module *curr = top;
  Module *next = nullptr;

  // search in hierarchy. use iterator in for loop. the following is common c++
  // dialect for iterator based loops
  for (auto nameIter = names.begin(); nameIter != names.end(); nameIter++) {
    // look for name in modules
    next = curr->getChildModule(*nameIter);
    if (!next) {
      break;
    }

    // move one layer deeper in hierarchy
    curr = next;
  }

  // if not found, curr is nullptr
  return (curr->getName() == target) ? curr : nullptr;
}

// given a top module and a cell's full name, this function traverses hierarchy
// until finding the cell and returns a pointer to it. Returns
// nullptr if search fails or input arguments are incorrect
// NOTE: fullname should not contain top module name
Cell *getCellFromFullName(Module *top, string fullName) {

  // return nullptr if starting module is not specified
  if (!top) {
    return nullptr;
  }

  // tokenize full name on '/'
  vector<string> names;
  boost::split(names, fullName, boost::is_any_of("/"), boost::token_compress_off);

  // if vector size is less than 2, cell is on top module
  if (names.size() < 2) {
    return top->getCell(fullName);
  }

  string &cellName = names[names.size() - 1];

  // find parent module
  Module *curr = getModuleFromFullName(top, fullName.substr(0, fullName.find_last_of('/')));
  // now, curr contains last module. get cell
  Cell *cell = curr->getCell(cellName);

  // return nullptr if cell not found
  if (!cell) {
    return nullptr;
  }

  return cell;
}

// given a top module and a port's full name, this function traverses hierarchy
// until finding the port and returns a pointer to it. Returns
// nullptr if search fails or input arguments are incorrect
// NOTE: fullname should not contain top module name
Port *getPortFromFullName(Module *top, string fullName) {

  // return nullptr if starting module is not specified
  if (!top) {
    return nullptr;
  }

  // tokenize full name on '/'
  vector<string> names;
  boost::split(names, fullName, boost::is_any_of("/"), boost::token_compress_off);

  // if vector size is less than 2, cell is on top module
  if (names.size() < 2) {
    return top->getPort(fullName);
  }

  // last name is pin name. use reference to avoid deep copy
  string &portName = names[names.size() - 1];

  Module *curr = getModuleFromFullName(top, fullName.substr(0, fullName.find_last_of('/')));

  Port *port = curr->getPort(portName);

  if (!port) {
    return nullptr;
  }

  return port;
}

// main data structure creation entry point. the following function contains an
// example of how to retrieve all you need form ASP data structures. Your code
// should go in this function too. Feel free to either keep the already existing
// printing or scrap it and use your own.
// NOTE: ASP-returned full names start with the top module name (on purpose) in
// case you want to exploit this information in any way. The standard EDA tools
// convention is that the top module name is omitted from all full names for
// efficiency. So for example a top module port's full name will be  mp1 instead
// of top/mp1. Except from ASP APIs, this project uses the standard convention,
// as shown in above functions.
int create_data_structures() {
  char *fullname = NULL;
  char *defname = NULL;
  char *libcellname = NULL;
  float x = 0;
  float y = 0;
  float h = 0;
  float w = 0;
  char direction = 0; // 1 for input, 2 for output, 3 for inout //
  char exists;

  if (mainLibrary == nullptr) {
    cout << "Error. Main Library is not loaded." << endl;
    return TCL_ERROR;
  }
  // print all modules
  #ifdef ENABLE_PRINTS 
    cout << "MODULES:" << endl;
  #endif

  while (ASP_get_next_module_name_and_hier_name(&defname, &fullname)) {

    if (defname && fullname) {
      #ifdef ENABLE_PRINTS
      printf(">module %s: %s\n", defname, fullname);
      #endif
    }
  }

  fullname = NULL;
  defname = NULL;
  // print all cells
  #ifdef ENABLE_PRINTS
    cout << "CELLS:" << endl;
  #endif 
  do {
    float h = 0;
    float w = 0;
    exists = ASP_get_next_cell_full_name_libcell_and_size(&fullname, &libcellname, &h, &w);

    if (exists) {
      #ifdef ENABLE_PRINTS 
        printf("%s %s h: %f, w: %f\n", libcellname, fullname, h, w);
      #endif
    }
  } while (exists);

  fullname = NULL;

  // print all pins
  #ifdef ENABLE_PRINTS 
    cout << "PINS:" << endl;
  #endif

  do {
    exists = ASP_get_next_pin_info(&fullname, &x, &y, &h, &w, &direction);

    if (exists) {
      #ifdef ENABLE_PRINTS
        printf("%s (%s) x: %f, y: %f, h: %f, w: %f\n", fullname,
              (direction == 1) ? "In" : "Out", x, y, h, w);
      #endif
    }
  } while (exists);

  fullname = NULL;

  #ifdef ENABLE_PRINTS 
    cout << "PORTS:" << endl;
  #endif
  // print all ports
  do {
    exists = ASP_get_next_port_info(&fullname, &x, &y, &h, &w, &direction);

    if (exists) {
      #ifdef ENABLE_PRINTS 
        printf("%s (%s) x: %f, y: %f, h: %f, w: %f\n", fullname,
              (direction == 1) ? "In" :
              (direction == 2) ? "Out" : "Inout", x, y, h, w);
      #endif
    }
  } while (exists);

  fullname = NULL;

  #ifdef ENABLE_PRINTS
    cout << "NETS:" << endl;
  #endif
  // print all nets
  do {
    char **con_names = NULL;
    int con_num = 0;
    exists = ASP_get_next_net_info(&fullname, &con_names, &con_num);

    if (exists) {
      #ifdef ENABLE_PRINTS 
        printf("%s \n", fullname);
      #endif
    }

    #ifdef ENABLE_PRINTS
      cout << "\tConnections Num " << con_num << endl;
      for (int i = 0; i < con_num; i++) {
        printf("\t\t%s\n", con_names[i]);
        free(con_names[i]);
    }
    #endif

    free(con_names);
    con_names = NULL;
  } while (exists);

  free(fullname);
  fullname = NULL;

  return TCL_OK;
}

// get all arcs between specified from to pins. returns a vector of pointers to LibTimingArcs
vector<LibTimingArc *> *getFromToTimingArcs(Pin *from_pin, Pin *to_pin) {
  if (!from_pin || !to_pin) {
    return nullptr;
  }

  LibPin *from_lpin = from_pin->getLibPin();
  LibPin *to_lpin = to_pin->getLibPin();

  assert(from_lpin->getType() == INPUT && "Error: 'from' pin has incorrect direction (expected INPUT). Program will exit.");
  assert(to_lpin->getType() == OUTPUT && "Error: 'to' pin has incorrect direction (expected OUTPUT). Program will exit.");
  

  TimingInfo *t_info = to_lpin->getTimingInfo();

  if (t_info->out_tinfo == nullptr) {
    return nullptr;
  }

  vector<LibTimingArc *> *vec = new vector<LibTimingArc *>();
  for (auto arc : t_info->out_tinfo->timing_arcs) {
    if (arc->getFromPin() == from_lpin) {
      (*vec).push_back(arc);
    }
  }
  return vec;
}
