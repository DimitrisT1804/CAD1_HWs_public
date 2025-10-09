//   ____    _    ____    ___    ____                            ____            _           _
//  / ___|  / \  |  _ \  |_ _|  / ___|___  _   _ _ __ ___  ___  |  _ \ _ __ ___ (_) ___  ___| |_
// | |     / _ \ | | | |  | |  | |   / _ \| | | | '__/ __|/ _ \ | |_) | '__/ _ \| |/ _ \/ __| __|
// | |___ / ___ \| |_| |  | |  | |__| (_) | |_| | |  \__ \  __/ |  __/| | | (_) | |  __/ (__| |_
//  \____/_/   \_\____/  |___|  \____\___/ \__,_|_|  |___/\___| |_|   |_|  \___// |\___|\___|\__|
//                                                                            |__/
// A project by University of Thessaly CASlab and Stavros Simoglou, for the purpose of academic
// training on the topics of physical design algorithms, EDA CAD, and larger project development
//
// File:    structs.hpp
// Purpose: data structures for design representation
// Authors: Stavros Simoglou
//
#ifndef _STRUCTS_DEF_HPP
#define _STRUCTS_DEF_HPP

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>
#include <assert.h>
#include <lib/lib.hpp>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <algorithm> 
#include <vector>
#include <utility>

using namespace std;

static bool natural_sorting(const std::string& a, const std::string& b) {
  // Find numeric suffix
  size_t i = 0;
  while (i < a.size() && !isdigit(a[i])) i++;
  size_t j = 0;
  while (j < b.size() && !isdigit(b[j])) j++;

  std::string prefixA = a.substr(0, i);
  std::string prefixB = b.substr(0, j);

  if (prefixA != prefixB)
      return prefixA < prefixB;

  int numA = (i < a.size()) ? std::stoi(a.substr(i)) : -1;
  int numB = (j < b.size()) ? std::stoi(b.substr(j)) : -1;
  return numA < numB;
}


template <typename Assoc>
static auto sorted_items(const Assoc& m) {
    using Pair = std::pair<typename Assoc::key_type, typename Assoc::mapped_type>;
    std::vector<Pair> v;
    v.reserve(m.size());
    for (const auto& kv : m) v.emplace_back(kv.first, kv.second);
    std::sort(v.begin(), v.end(),
          [](const Pair& a, const Pair& b) { return natural_sorting(a.first, b.first); });
    return v;
}

// all objects inherit from class Box. Given that the project is
// for the physical design course, it is safe to assume that
// all shapes are rectangles/boxes
class Box {
public:
  typedef enum {
    BOTTOM_LEFT = 0,
    TOP_LEFT,
    BOTTOM_RIGHT,
    TOP_RIGHT
  } OriginType;

  // default constructor
  Box() : mX(0.0), mY(0.0), mH(0.0), mW(0.0), mO(BOTTOM_LEFT) {}

  Box(float x, float y, float h, float w, OriginType o) {
    mX = x;
    mY = y;
    mH = h;
    mW = w;
    mO = o;
  }

  void getCoordinates(float *x, float *y, float *h, float *w, OriginType *o) {
    if (x) {
      *x = mX;
    }
    if (y) {
      *y = mY;
    }
    if (h) {
      *h = mH;
    }
    if (w) {
      *w = mW;
    }
    if (o) {
      *o = mO;
    }
  }

  void setCoordinates(float *x, float *y, float *h, float *w, OriginType *o) {
    if (x) {
      mX = *x;
    }
    if (y) {
      mY = *y;
    }
    if (h) {
      mH = *h;
    }
    if (w) {
      mW = *w;
    }
    if (o) {
      mO = *o;
    }
  }
  void print() {
    cout << "Box" << endl;
    cout << "\tX: " << mX << endl;
    cout << "\tY: " << mY << endl;
    cout << "\tH: " << mH << endl;
    cout << "\tW: " << mW << endl;
    cout << "\tOrigin: " << mO << endl;

  }
private:
  float mX;      // member X coordinate
  float mY;      // member Y coordinate
  float mH;      // member Height
  float mW;      // member Width
  OriginType mO; // member Origin
};

// forward declaration of class Net. required for
// using pointers to Net in Pin and Port
class Net;
// forward declaration for Cell and Module, for up-casting from Instance to Cell
// and Module
class Module;
class Cell;
// forward declaration of pin needed in Cell
class Pin;

class Port;

extern Module *topModule;

// this union type is used to downcast from class Instance to Module or Cell
typedef union InstanceRefU {
  Module *moduleRef;
  Cell *cellRef;

  InstanceRefU() {
    moduleRef = nullptr;
    cellRef   = nullptr;
  }
} InstanceRef;

// Instances are Boxes with name. Modules and Cells inherit
// from class Instance (as in verilog)
class Instance : public Box {
public:
  typedef enum { NONE = 0, MODULE = 1, CELL } InstanceType;

  // default constructor
  Instance() : mName(), mParent(nullptr), mType(NONE) {}

  // no physical information constructor
  Instance(string name, Instance *parent, InstanceType type, InstanceRef objectReference) {
    mName   = name;
    mParent = parent;
    mType   = type;
    mRef    = objectReference;
  }

  // physical information constructor
  Instance(string name, Instance *parent, InstanceType type, InstanceRef objectReference, float x, float y,
           float h, float w, OriginType o) {
    mName   = name;
    mParent = parent;
    mType   = type;
    mRef    = objectReference;
    setCoordinates(&x, &y, &h, &w, &o);
  }

  // NOTE: no need to add explicit destructor. mParent is a pointer to allocated
  // memory which is freed when the pointed instance is free'd and strings are
  // auto-destructed

  void setInstanceInfo(string name, Instance *parent, InstanceType type,
                       InstanceRef objectReference) {
    mName = name;
    mParent = parent;
    mType = type;
    mRef = objectReference;
  }

  // getters
  string &getName() { return mName; }
  Instance *getInstance() { return this; }
  Instance *getParent() { return mParent; }

  void print() {
    cout << "Instance Name: " << mName << endl;
    string parentName = mParent ? mParent->getName() : "null";
    string strType;
    switch (mType) {
    case NONE: {
      strType = "NONE";
      break;
    }
    case MODULE: {
      strType = "MODULE";
      break;
    }
    case CELL: {
      strType = "CELL";
      break;
    }
    }
    cout << "\tParent Name: " << parentName << endl;
    cout << "\tInstance Type: " << strType << endl;
  }
  InstanceRef &getRef() {return mRef;}
private:
  string mName;       // instance name
  Instance *mParent;  // parent instance
  InstanceType mType; // instance type
  InstanceRef mRef;   // pointer to module or cell
};


// Nets are describing connectivity. Inherit box
// class as it can be used to store bounding box information
// no need to implement destructor. default destructor will clear the vectors
// and string and the pointed object will be free'd upon module/cell cleanup
class Net : Box {
public:
  // default constructor
  Net() {}

  // bounding box extension helper template
  template <class T>
  void extendNetBB(T *obj, float &X1, float &X2, float &Y1, float &Y2) {

    // temp object info
    float tmpX1, tmpY1, tmpX2, tmpY2, tmpH, tmpW;
    Box::OriginType tmpO;

    // get object bounding box coordinates
    obj->getCoordinates(&tmpX1, &tmpY1, &tmpH, &tmpW, &tmpO);

    tmpX2 = tmpX1 + tmpW;
    tmpY2 = tmpY1 + tmpH;
    X1 = min(X1, tmpX1);
    Y1 = min(Y1, tmpY1);
    X2 = max(X2, tmpX2);
    Y2 = max(Y2, tmpY2);
  }

  // bounding box extension  template overload using 'this'
  template <class T>
  void extendNetBB(T *obj) {

    float X1 = 0.0; // bottom left
    float Y1 = 0.0; // bottom left
    float H  = 0.0; // height
    float W  = 0.0; // width
    float X2 = 0.0; // top right
    float Y2 = 0.0; // top right
    Box::OriginType O;

    // get net bounding box
    this->getCoordinates(&X1, &Y1, &H, &W, &O);;

    // calculate net bounding box X2, Y2
    X2 = X1 + W;
    Y2 = Y1 + H;

    extendNetBB(obj, X1, Y1, X2, Y2);

    // calculate height and width and set bounding box info
    W = X2 - X1;
    H = Y2 - Y1;
    this->setCoordinates(&X1, &Y1, &H, &W, &O);
  }

  // constructor. it's handy to extend the bounding box here
  Net(string name, vector<Pin *> pins, vector<Port *> ports) {
    mName = name;
    mPins = pins;
    mPorts = ports;

    // bounding box information. is orientation needed?
    float X1 = 0.0; // bottom left
    float Y1 = 0.0; // bottom left
    float X2 = 0.0; // top right
    float Y2 = 0.0; // top right

    // extend bounding box for pins
    for (auto pin : pins) {
      extendNetBB(pin, X1, X2, Y1, Y2);
    }

    // extend bounding box for ports
    for (auto port : ports) {
      extendNetBB(port, X1, X2, Y1, Y2);
    }

    // calculate bounding box height and width from X1, Y1 and X2, Y2
    float H = Y2 - Y1;
    float W = X2 - X1;

    Box::OriginType tmpO = Box::BOTTOM_LEFT;
    setCoordinates(&X1, &Y1, &H, &W, &tmpO);
  }

  void insertPin(Pin *pin) {
    if (pin == nullptr) {
      return;
    }

    // extend bounding box
    extendNetBB(pin);
    // insert pin
    mPins.push_back(pin);
  }

  void insertPort(Port *port) {
    if (port == nullptr) {
      return;
    }

    // extend bounding box
    extendNetBB(port);
    // insert port
    mPorts.push_back(port);
  }

  string &getName() {
    return mName;
  }
private:
  string mName;          // Net name
  vector<Pin *> mPins;   // pointers to pins
  vector<Port *> mPorts; // pointers to ports
};


// Cells exist in Modules. As with Ports, the X, Y must be stored
// w.r.t. Module origin
class Cell : public Instance {
public:
  // default constructor
  Cell() {};

  // no physical information constructor; declaration is here, implementation is in
  // structs.cpp because we need to access mainLibrary pointer
  Cell(string name, string libcell_name, Instance *parent);

  // physical information constructor; declaration is here, implementation is in
  // structs.cpp because we need to access mainLibrary pointer
  Cell(string name, string libcell_name, Instance *parent, float x, float y,
       float h, float w, OriginType o);

  // same with destructor and printer
  ~Cell();
  void print();

  void insertPin(string name, Pin *pin) {
    // create std::pair for unordered_map insertion
    pair<string, Pin*> entry(name, pin);
    mPins.insert(entry);
  }

  // getters
  Pin *getPin(string &name) {
    // look for object. If did not found it return end iterator, so return
    // nullptr in this case
    auto iter = mPins.find(name);
    if (iter == mPins.end()) {
      return nullptr;
    }

    return iter->second;
  }

  LibCell *getLibCell() {return mLibcell;}

  void write_netlist();

private:
  // cells typically contain small amount of pins. use
  // map (balanced binary search tree) for memory and
  // lookup efficiency
  map<string, Pin *> mPins; // pins list
  LibCell *mLibcell; // pointer to lib cell for fast access
};
  
  
// Pins exist in Cells. They still inherit Box class as they are described
// by rectangles. Note that to ensure they always exist within the parent
// Cell, their X and Y must be stored w.r.t. parent Cell's origin coordinates.
class Pin : public Box {
public:
  // default constructor
  Pin() : mName(), mParent(nullptr), mNet(nullptr), mType(NONE) {}

  // no physical information constructor
  Pin(string name, Instance *parent, Net *net, DirectionType type) {
    mName   = name;
    mParent = parent;
    mNet    = net;
    mType = type;

    Cell *parent_cell = parent->getRef().cellRef;
    mLibpin = parent_cell->getLibCell()->getPin(name);
  }

  // physical information constructor
  Pin(string name, Instance *parent, Net *net, DirectionType type, float x, float y, float h,
      float w, OriginType o) {
    mName   = name;
    mParent = parent;
    mNet    = net;
    mType = type;
    setCoordinates(&x, &y, &h, &w, &o);
    Cell *parent_cell = parent->getRef().cellRef;
    mLibpin = parent_cell->getLibCell()->getPin(name);
  }

  // auto destructor
  ~Pin() {}

  // getters
  string getName() { return mName; }
  Instance *getParent() { return mParent; }
  Net *getNet() { return mNet; }

  void insertNet(Net *net) { mNet = net; }

  void print() {
    cout << "\tPin: " << mName << endl;
    cout << "\tParent: " << mParent->getName() << endl;
    cout << "\tType: " << mType << endl;
    cout << "-----------" << endl;
  }

  void write_netlist() {
    // Print the pin connection in the format .PinName() //
    Net *net = this->getNet();
    if (net == nullptr) {
      cout << "." << this->getName() << "(null)";
    }
    else {
      cout << "." << this->getName() << "(" << net->getName() << ")";
    }
  }

private:
  string mName;        // Pin name
  Instance *mParent;   // parent Instance pointer
  Net *mNet;           // connected net
  DirectionType mType; // pin direction
  LibPin *mLibpin; // pointer to libpin for fast access
};

  
// Ports exist in Modules. As with Pins and Cells, the X, Y must be stored
// w.r.t. Module origin
class Port : public Box {
public:
  // default constructor
  Port() : mName(), mParent(nullptr), mNets(), mType(NONE) {}

  // no physical information constructor
  Port(string name, Instance *parent, Net *driver_net, Net *driven_net, DirectionType type) {
    mName   = name;
    mParent = parent;
    mType   = type;
    if (driver_net) {
      mNets.push_back(driver_net);
    }
    if (driven_net) {
      mNets.push_back(driven_net);
    }
  }

  // physical information constructor
  Port(string name, Instance *parent, Net *driver_net, Net *driven_net, DirectionType type, float x, float y, float h,
      float w, OriginType o) {
    mName   = name;
    mParent = parent;
    mType   = type;
    if (driver_net) {
      mNets.push_back(driver_net);
    }
    if (driven_net) {
      mNets.push_back(driven_net);
    }
    setCoordinates(&x, &y, &h, &w, &o);
  }

  // getters
  string getName() { return mName; }
  Instance *getParent() { return mParent; }
  DirectionType getType() { return mType; }

  void insertNet(Net *netPtr) {
    // Check for nullptr before pushing to avoid issues
    if (netPtr) {
      // Use push_back to add the new Net pointer to the vector of nets
      mNets.push_back(netPtr);
    }
  }

  vector <Net *> &getNets() { return mNets; }

  // void insertNet(Net *net) { mNets = net; }

private:
  string mName;        // Port name
  Instance *mParent;   // parent Instance pointer
  vector<Net *> mNets; // connected nets; module ports may have 2 nets attached
                       // to them, one drives them and one is driven by them
  DirectionType mType; // pin direction
};

// Modules are the highest placeable Instances (objects) in Verilog hierarchy.
// They store Ports, Cells, Nets and other Modules. Parent Module is known
// through Instance parent class, while children modules are stored. That way,
// verilog module tree hierarchy is implicitly created. Also X Y coordinates
// must be stored w.r.t. parent module origin coordinates
class Module : public Instance {
public:
  // default constructor
  Module() {}

  // no physical information constructor
  Module(string defname, string name, Instance *parent) {
    InstanceRef ref;
    ref.moduleRef = this;
    setInstanceInfo(name, parent, Instance::MODULE, ref);
    mDefName = defname;
  }

  // physical information constructor
  Module(string defname, string name, Instance *parent, float x, float y, float h, float w,
         Box::OriginType o) {
    InstanceRef ref;
    ref.moduleRef = this;

    setInstanceInfo(name, parent, Instance::MODULE, ref);
    setCoordinates(&x, &y, &h, &w, &o);
    mDefName = defname;
  }

  // destructor to clean cells, modules, ports, and nets
  ~Module() {
    // always call the destructor to not cause any memory leaks from leftover
    // pointers

    // use ranged for to free all cells
    for (auto entry : mCells) {
      delete entry.second;
    }

    // use ranged for to free all modules
    for (auto entry : mModules) {
      delete entry.second;
    }

    // use ranged for to free all ports
    for (auto entry : mPorts) {
      delete entry.second;
    }

    // use ranged for to free all nets
    for (auto entry : mNets) {
      delete entry.second;
    }
  }

  // methods for data insertion
  void insertCell(string name, Cell *cell) {
    // create std::pair for unordered_map insertion
    pair<string, Cell*> entry(name, cell);
    mCells.insert(entry);
  }
  void insertPort(string name, Port *port) {
    // create std::pair for unordered_map insertion
    pair<string, Port*> entry(name, port);
    mPorts.insert(entry);
  }
  void insertModule(string name, Module *module) {
    // create std::pair for unordered_map insertion
    pair<string, Module*> entry(name, module);
    mModules.insert(entry);
  }
  void insertNet(string name, Net *net) {
    // create std::pair for unordered_map insertion
    pair<string, Net*> entry(name, net);
    mNets.insert(entry);
  }

  // getters. could be templated but let's not get too extreme with c++ little
  // helpers
  Module *getChildModule(string &name) {
    // look for object. If not found it return end iterator, so return nullptr
    // in this case
    auto iter = mModules.find(name);
    if (iter == mModules.end()) {
      return nullptr;
    }

    return iter->second;
  }
  Cell *getCell(string &name) {
    // look for object. If not found it return end iterator, so return nullptr
    // in this case
    auto iter = mCells.find(name);
    if (iter == mCells.end()) {
      return nullptr;
    }

    return iter->second;
  }
  Port *getPort(string &name) {
    // look for object. If not found it return end iterator, so return nullptr
    // in this case
    auto iter = mPorts.find(name);
    if (iter == mPorts.end()) {
      return nullptr;
    }

    return iter->second;
  }
  Net *getNet(string &name) {
    // look for object. If not found it return end iterator, so return nullptr
    // in this case
    auto iter = mNets.find(name);
    if (iter == mNets.end()) {
      return nullptr;
    }

    return iter->second;
  }

  void print() {
    Instance *inst = getInstance();
    cout << "Module " << inst->getName() << endl;
    cout << "Instance: " << endl;
    inst->print();

    // print cells
    cout << "Cells: " << endl;
    for (auto entry : mCells) {
      entry.second->print();
    }
  }

  void write_netlist(string filename) {
    int i = 0;
    ofstream file;
    streambuf *coutbuf;

    if (this == topModule && !filename.empty()) {
      file.open(filename);
      coutbuf = cout.rdbuf(); // save old buffer //
      cout.rdbuf(file.rdbuf());         // redirect std::cout to file //

      auto now = chrono::system_clock::now();
      time_t now_c = chrono::system_clock::to_time_t(now);
      tm *local_time = localtime(&now_c);
      cout << "// Generated netlist by CADI API on Date: " << put_time(local_time, "%Y-%m-%d %H:%M:%S") << " //" << endl << endl;
    }

    cout << "module " << this->mDefName << " (";

    i = 0;
    for (const auto& [pname, pptr] : sorted_items(mPorts)) {
      if (i == 0) {
        cout << pptr->getName();
      }
      else {
        cout << ", " << pptr->getName();;
      }
      i++;
    }
    cout << ");" << endl;
    
    i = 0;
    if (mCells.size() > 0) {
      cout << "input ";

      for (const auto& [pname, pptr] : sorted_items(mPorts)) {
        if (pptr->getType() == INPUT) {
          if (i != 0) std::cout << ", ";
          std::cout << pname;
          i++;
        }
      }
      cout << ";" << endl;
      i = 0;
  
      cout << "output "; 
      for (const auto& [pname, pptr] : sorted_items(mPorts)) {
        if (pptr->getType() == OUTPUT) {
          if (i != 0) std::cout << ", ";
          std::cout << pname;
          i++;
        }
      }
      cout << ";" << endl;
      cout << endl;
    }


    // --- Cells (sorted by instance name) ---
    for (const auto& [cname, cptr] : sorted_items(mCells)) {
      (void)cname; // not used here, but kept for symmetry
      cptr->write_netlist();
    }

    // --- Submodules as instances (sorted by instance name) ---
    for (const auto& [mname, mptr] : sorted_items(mModules)) {
    (void)mname;
    mptr->print_as_instance(this);
    }

    cout << "endmodule" << endl << endl;

    for (auto entry : mModules) {
      entry.second->write_netlist("");
    }

    if (this == topModule && !filename.empty()) {
      cout.rdbuf(coutbuf); // reset to standard output again //
      file.close();
    }
  }

  void print_as_instance(Module *parentmodule) {
    Instance *inst = getInstance();
    int i = 0;

    // stdout << entry.first << " " << entry.first << "_inst()";
    cout << this->mDefName << " " << inst->getName();
    cout << "(";
    
    for (const auto& [pname, pptr] : sorted_items(mPorts)) {
      if (i == 0) {
        cout << "." << pptr->getName() << "(";
      }
      else {
        cout << ", ." << pptr->getName() << "(";
      }
      i++;
      for (auto net : pptr->getNets()) {
        Net *portnet = parentmodule->getNet(net->getName());
        if (portnet){
          cout << portnet->getName();
          break;
        }
      }
      cout << ")";
    }
    cout << ");" << endl;
  }


private:
  string mDefName; // module definition name
  // use unordered map (hash) for faster cell lookup
  unordered_map<string, Cell *> mCells; // cells list
  // use unordered map for faster module lookup
  unordered_map<string, Module *> mModules; // children modules list
  // use map for ports. typically sorter names
  map<string, Port *> mPorts; // ports list
  // use map for nets. typically sorter names. also
  // vector can be used as they are not usually
  // looked up by name
  map<string, Net *> mNets; // nets list
};

#endif
