#include <structs_def.hpp>

extern Pin *getPinFromFullName(Module *module1, string fullName);
extern Port *getPortFromFullName(Module *module1, string fullName);
extern Module *getModuleFromFullName(Module *module1, string fullName);
extern string getFullNameFromPin(Pin *pin);
extern Cell *getCellFromFullName(Module *top, string fullName);
extern char ASP_get_next_module_full_name(char *fullname);

// create a simple hierarchy like top/module1, top/module2, top/module1/module3.
// module1 cells: cell1, cell2
// module2 cells: cell1, cell2, cell3
// module3 cells: cell1
// cell1 pins: A B X (sg13g2_and2_1)
// cell2 pins: A Z (sg13g2_einvn_2)
// cell3 pins: A B Y (sg13g2_nand2_1)
// module1 ports: in1, in2, out1, out2
// module2 ports: in1, in2, out
// module3 ports: in1, out
void unit_test_structs() {

  Net *net;
  Port *port_con;
  Pin *pin_con;

  topModule = new Module("top", "top", nullptr, 0, 0, 100, 100, Box::BOTTOM_LEFT);

  Module *module1;
  module1 = new Module("module1", "module1_inst", topModule->getInstance(), 0, 10, 10, 10, Box::BOTTOM_LEFT);
  topModule->insertModule(module1->getName(), module1);

  // ports
  Port *port = new Port("in1", module1, nullptr, nullptr, INPUT);
  module1->insertPort(port->getName(), port);
  port = nullptr;
  port = new Port("in2", module1, nullptr, nullptr, INPUT);
  module1->insertPort(port->getName(), port);
  port = new Port("out1", module1, nullptr, nullptr, OUTPUT);
  module1->insertPort(port->getName(), port);
  port = new Port("out2", module1, nullptr, nullptr, OUTPUT);
  module1->insertPort(port->getName(), port);

  // create cell1
  Cell *cell = new Cell("cell1", "sg13g2_and2_1", module1->getInstance());
  module1->insertCell(cell->getName(), cell);


  // pin's parent instance is cell
  Instance *instance = cell->getInstance();
  Pin *pin = new Pin("A", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("B", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("X", instance, nullptr, OUTPUT);
  cell->insertPin(pin->getName(), pin);

  // create cell2
  cell = new Cell("cell2", "sg13g2_einvn_2", module1->getInstance());
  module1->insertCell(cell->getName(), cell);
  instance = cell->getInstance();

  // create cell2 pins
  pin = new Pin("A", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("Z", instance, nullptr, OUTPUT);
  cell->insertPin(pin->getName(), pin);

  // create module2 inside top module //
  Module *module2 = new Module("module2", "module2_inst", topModule->getInstance(), 0, 10, 10, 10, Box::BOTTOM_LEFT);
  topModule->insertModule(module2->getName(), module2);

  // ports module 2 //
  port = new Port("in1", module2, nullptr, nullptr, INPUT);
  module2->insertPort(port->getName(), port);
  port = nullptr;
  port = new Port("in2", module2, nullptr, nullptr, INPUT);
  module2->insertPort(port->getName(), port);
  port = new Port("out", module2, nullptr, nullptr, OUTPUT);
  module2->insertPort(port->getName(), port);

  // create cell1 for module2 //
  cell = new Cell("cell1", "sg13g2_and2_1", module2->getInstance());
  module2->insertCell(cell->getName(), cell);
  instance = cell->getInstance();
  pin = new Pin("A", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("B", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("X", instance, nullptr, OUTPUT);
  cell->insertPin(pin->getName(), pin);

  // create cell2 for module2 //
  cell = new Cell("cell2", "sg13g2_einvn_2", module2->getInstance());
  module2->insertCell(cell->getName(), cell);
  instance = cell->getInstance();
  pin = new Pin("A", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("Z", instance, nullptr, OUTPUT);
  cell->insertPin(pin->getName(), pin);

  // create cell3 for module2 //
  cell = new Cell("cell3", "sg13g2_nand2_1", module2->getInstance());
  module2->insertCell(cell->getName(), cell);
  instance = cell->getInstance();
  pin = new Pin("A", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("B", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("Y", instance, nullptr, OUTPUT);
  cell->insertPin(pin->getName(), pin);


  // create module3 inside module1 //
  Module *module3 = new Module("module3", "module3_inst", module1->getInstance(), 0, 10, 10, 10, Box::BOTTOM_LEFT);
  module1->insertModule(module3->getName(), module3);

  Port *port3 = new Port("in1", module3, nullptr, nullptr, INPUT);
  module3->insertPort(port3->getName(), port3);
  port3 = nullptr;
  port3 = new Port("out", module3, nullptr, nullptr, OUTPUT);
  module3->insertPort(port3->getName(), port3);

  // create cell1 for module3 //
  cell = new Cell("cell1", "sg13g2_and2_1", module3->getInstance());
  module3->insertCell(cell->getName(), cell);
  instance = cell->getInstance();
  pin = new Pin("A", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("B", instance, nullptr, INPUT);
  cell->insertPin(pin->getName(), pin);
  pin = new Pin("X", instance, nullptr, OUTPUT);
  cell->insertPin(pin->getName(), pin);


  // create nets inside top module //
  net = new Net("net1", {}, {});
  topModule->insertNet(net->getName(), net);
  port_con = getPortFromFullName(topModule, "module1_inst/in1");
  port_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module2_inst/in1");
  port_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/out2");
  port_con->insertNet(net);

  net = new Net("net2", {}, {});
  topModule->insertNet(net->getName(), net);
  port_con = getPortFromFullName(topModule, "module1_inst/in2");
  port_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module2_inst/in2");
  port_con->insertNet(net);

  net = new Net("net3", {}, {});
  topModule->insertNet(net->getName(), net);
  port_con = getPortFromFullName(topModule, "module1_inst/out1");
  port_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module2_inst/out");
  port_con->insertNet(net);


  // create nets for module1 //
  net = new Net("in1", {}, {});
  module1->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module1_inst/cell1/A");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/in1");
  port_con->insertNet(net);

  net = new Net("in2", {}, {});
  module1->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module1_inst/cell1/B");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/in2");
  port_con->insertNet(net);

  // net 3 //
  net = new Net("net1", {}, {});
  module1->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module1_inst/cell1/X");
  pin_con->insertNet(net);
  pin_con = getPinFromFullName(topModule, "module1_inst/cell2/A");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/module3_inst/in1");
  port_con->insertNet(net);

  // net 4 //
  net = new Net("out1", {}, {});
  module1->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module1_inst/cell2/Z");
  pin_con->insertNet(net);  
  port_con = getPortFromFullName(topModule, "module1_inst/out1");
  port_con->insertNet(net);

  // net 5 //
  net = new Net("out2", {}, {});
  module1->insertNet(net->getName(), net);
  port_con = getPortFromFullName(topModule, "module1_inst/module3_inst/out");
  port_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/out2");
  port_con->insertNet(net);


  // create nets for module2 //
  net = new Net("in1", {}, {});
  module2->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module2_inst/cell1/A");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module2_inst/in1");
  pin_con = getPinFromFullName(topModule, "module2_inst/cell3/B");
  pin_con->insertNet(net);

  net = new Net("in2", {}, {});
  module2->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module2_inst/cell1/B");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module2_inst/in2");
  port_con->insertNet(net);

  // net 3 //
  net = new Net("net1", {}, {});
  module2->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module2_inst/cell1/X");
  pin_con->insertNet(net);
  pin_con = getPinFromFullName(topModule, "module2_inst/cell2/A");
  pin_con->insertNet(net);  

  // net 4 //
  net = new Net("net2", {}, {});
  module2->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module2_inst/cell2/Z");
  pin_con->insertNet(net);  
  pin_con = getPinFromFullName(topModule, "module2_inst/cell3/A");
  pin_con->insertNet(net);


  // net 6 //
  net = new Net("out", {}, {});
  module2->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module2_inst/cell3/Y");
  pin_con->insertNet(net);  
  port_con = getPortFromFullName(topModule, "module2_inst/out");
  port_con->insertNet(net);

  // create nets for module3 //
  net = new Net("in1", {}, {});
  module3->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module1_inst/module3_inst/cell1/A");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/module3_inst/in1");
  port_con->insertNet(net);
  pin_con = getPinFromFullName(topModule, "module1_inst/module3_inst/cell1/B");
  pin_con->insertNet(net);

  net = new Net("out", {}, {});
  module3->insertNet(net->getName(), net);
  pin_con = getPinFromFullName(topModule, "module1_inst/module3_inst/cell1/X");
  pin_con->insertNet(net);
  port_con = getPortFromFullName(topModule, "module1_inst/module3_inst/out");
  port_con->insertNet(net);
  

  // print netlist for validation //
  cout << "----------------" << endl;
  cout << "Netlsit of test " << endl;
  topModule->write_netlist("");


  cout << "----------------" << endl;
  cout << "expected module is top Module :" << endl;
  getModuleFromFullName(topModule, "")->print();

  cout << "----------------" << endl;
  cout << "expected module is module1 (module1) :" << endl;
  getModuleFromFullName(topModule, "module1_inst")->print();

  cout << "----------------" << endl;
  cout << "expected module is top/module2_inst  :" << endl;
  getModuleFromFullName(topModule, "module2_inst")->print();
  cout << "----------------" << endl;
  cout << "expected module is top/module1_inst/module3_inst  :" << endl;
  getModuleFromFullName(topModule, "module1_inst/module3_inst")->print();
  cout << "----------------" << endl;
  cout << "expected cell is module1_inst/cell1  :" << endl;
  getCellFromFullName(topModule, "module1_inst/cell1")->print();
  cout << "----------------" << endl;
  cout << "expected Port is module1_inst/module3_inst/in1  :" << endl;
  Port *newport = getPortFromFullName(topModule, "module1_inst/module3_inst/in1");
  cout << "Port is: " << newport->getName() << endl;
  cout << "----------------" << endl;



  delete topModule;
}
