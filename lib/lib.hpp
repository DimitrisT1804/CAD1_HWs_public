//   ____    _    ____    ___ ___    ____                            ____            _           _
//  / ___|  / \  |  _ \  |_ _|_ _|  / ___|___  _   _ _ __ ___  ___  |  _ \ _ __ ___ (_) ___  ___| |_
// | |     / _ \ | | | |  | | | |  | |   / _ \| | | | '__/ __|/ _ \ | |_) | '__/ _ \| |/ _ \/ __| __|
// | |___ / ___ \| |_| |  | | | |  | |__| (_) | |_| | |  \__ \  __/ |  __/| | | (_) | |  __/ (__| |_
//  \____/_/   \_\____/  |___|___|  \____\___/ \__,_|_|  |___/\___| |_|   |_|  \___// |\___|\___|\__|
//                                                                                |__/
// A project by University of Thessaly CASlab and Stavros Simoglou, for the purpose of academic
// training on the topics of physical design algorithms, EDA CAD, and larger project development
//
// File:    lib.hpp
// Purpose: data structures for library info
// Authors: Stavros Simoglou, Dimitris Tsalapatasu
//
#ifndef _LIB_HPP
#define _LIB_HPP

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <si2dr_liberty.h>
#include <cstring>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std;

typedef enum {NONE = 0, INPUT = 1, OUTPUT = 2, INOUT = 3} DirectionType;
typedef enum {RISE = 0, FALL = 1} RFType;

typedef enum {CELL_RISE       = 0,
              CELL_FALL       = 1,
              RISE_TRANSITION = 2,
              FALL_TRANSITION = 3} LUTType;

typedef enum {COMBINATIONAL      = 1,
              COMBINATIONAL_RISE = 2,
              COMBINATIONAL_FALL = 3,
              RISING_EDGE        = 4,
              FALLING_EDGE       = 5} TimingType;

typedef enum {POSITIVE_UNATE = 1, NEGATIVE_UNATE = 2, NON_UNATE = 3} TimingSense;
typedef enum {INPUT_SLEW = 1, OUTPUT_LOAD = 2} LUTVariableType;
typedef liberty_value_data LUTDataType;
inline TimingType stringToTimingType(string type_str) {
  if (type_str == "combinational") {
    return COMBINATIONAL;
  } else if (type_str == "combinational_rise") {
    return COMBINATIONAL_RISE;
  } else if (type_str == "combinational_fall") {
    return COMBINATIONAL_FALL;
  } else if (type_str == "rising_edge") {
    return RISING_EDGE;
  } else if (type_str == "falling_edge") {
    return FALLING_EDGE;
  } else {
    // default is combinational
    return (TimingType)COMBINATIONAL;
  }
}

inline TimingSense stringToTimingSense(string sense_str) {
  if (sense_str == "positive_unate") {
    return POSITIVE_UNATE;
  } else if (sense_str == "negative_unate") {
    return NEGATIVE_UNATE;
  } else if (sense_str == "non_unate") {
    return NON_UNATE;
  } else {
    return (TimingSense)NONE;
  }
}

inline const char *directionToString(DirectionType direction) {
  switch (direction) {
  case INPUT:
    return "INPUT";
  case OUTPUT:
    return "OUTPUT";
  case INOUT:
    return "INOUT";
  default:
    return "NONE";
  }
}
inline const char *timingTypeToString(TimingType type) {
  switch (type) {
  case COMBINATIONAL:
    return "COMBINATIONAL";
  case COMBINATIONAL_RISE:
    return "COMBINATIONAL_RISE";
  case COMBINATIONAL_FALL:
    return "COMBINATIONAL_FALL";
  case RISING_EDGE:
    return "RISING_EDGE";
  case FALLING_EDGE:
    return "FALLING_EDGE";
  default:
    return "NONE";
  }
}

inline const char *timingSenseToString(TimingSense sense) {
  switch (sense) {
  case POSITIVE_UNATE:
    return "POSITIVE_UNATE";
  case NEGATIVE_UNATE:
    return "NEGATIVE_UNATE";
  case NON_UNATE:
    return "NON_UNATE";
  default:
    return "NONE";
  }
}

inline LUTVariableType stringToLUTVariableType(string var_type_str) {
  if (var_type_str == "input_net_transition") {
    return INPUT_SLEW;
  } else if (var_type_str == "total_output_net_capacitance") {
    return OUTPUT_LOAD;
  } else {
    return (LUTVariableType)NONE;
  }
}

inline const char *LUTVariableTypeToString(LUTVariableType var_type) {
  switch (var_type) {
  case INPUT_SLEW:
    return "input_net_transition";
  case OUTPUT_LOAD:
    return "total_output_net_capacitance";
  default:
    return "NONE";
  }
}

inline const char *LUTTypeToString(LUTType type) {
  switch (type) {
  case CELL_RISE:
    return "cell_rise";
  case CELL_FALL:
    return "cell_fall";
  case RISE_TRANSITION:
    return "rise_transition";
  case FALL_TRANSITION:
    return "fall_transition";
  default:
    return "NONE";
  }
}


class Lib;
class LibCell;
class LibPin;
class LibTimingArc;

typedef struct FF_GROUP {
  LibPin *clocked_on_pin;
  LibPin *preset_pin;
  LibPin *clear_pin;
} ff_group;  

class LUTTemplate {
  public:
    LUTTemplate(string name) {
      m_name = name;
    }
    string &getName() {
      return m_name;
    }
  
    void setVar1(LUTVariableType var) { m_variable1 = var; }
    void setVar2(LUTVariableType var) { m_variable2 = var; }
    vector<long double> *getIndex1() { return &m_index1; }
    vector<long double> *getIndex2() { return &m_index2; }
    LUTVariableType getVar1() { return m_variable1; }
    LUTVariableType getVar2() { return m_variable2; }
    void print() {
      cout << "Template: " << m_name << endl;
      cout << "Variable_1: " << LUTVariableTypeToString(m_variable1) << endl;
      cout << "Variable_2: " << LUTVariableTypeToString(m_variable2) << endl;
      cout << "Index_1: ";
      for (auto val : m_index1) {
        cout << val << ", ";
      }
      cout << endl << "Index_2: ";
      for (auto val : m_index2) {
        cout << val << ", ";
      }
      cout << endl;
    }
  
  private:
    LUTVariableType m_variable1;
    LUTVariableType m_variable2;
    vector<long double> m_index1; // long double is the default index type in lib parser
    vector<long double> m_index2; // long double is the default index type in lib parser
    string m_name;
  };

class LUT {
public:
  LUT(LUTType type, LUTDataType *data, LUTTemplate *lut_template) {
    m_type = type;
    m_data = data;
    m_lut_template = lut_template;
  }

  ~LUT() {
    if (m_data) {
      liberty_destroy_value_data(m_data);
    }
  }
  LUTDataType *getLUTData() { return m_data; }
  LUTTemplate *getTemplate() { return m_lut_template; }
  void print() {
    cout << LUTTypeToString(m_type) << " " << m_lut_template->getName() << endl;
  }
  LUTType getLUTType() { return m_type; }

private:
  LUTType      m_type;
  LUTDataType *m_data;
  LUTTemplate *m_lut_template;
};



class LibTimingArc {
public:
  LibTimingArc(LibPin *from_pin, LibPin *to_pin) {
    m_from_pin = from_pin;
    m_to_pin = to_pin;
  }

  LibTimingArc(LibPin *to_pin) {
    m_to_pin = to_pin;
  }

  void insertFromPin(LibPin *from_pin) { m_from_pin = from_pin; }
  void insertToPin(LibPin *to_pin) { m_to_pin = to_pin; }
  void insertTimingType(TimingType timing_type) { m_timing_type = timing_type; }
  void insertWhen(string when) { m_when = when; }
  void insertTimingSense(TimingSense timing_sense) {
    m_timing_sense = timing_sense;
  }
  void insertLUT(LUT *lut) {
    if (lut) {
      m_LUTs.push_back(lut);
    }
  }

  // assignment operator
  LibTimingArc &operator=(const LibTimingArc &rhs) {
    m_from_pin     = rhs.m_from_pin; // related_pin
    m_to_pin       = rhs.m_to_pin;
    m_timing_type  = rhs.m_timing_type;
    m_timing_sense = rhs.m_timing_sense;
    m_when         = rhs.m_when;
    m_LUTs         = rhs.m_LUTs;
    return *this;
  }

  // copy constructor
  LibTimingArc(const LibTimingArc &rhs) { *this = rhs; }

  void print();

  ~LibTimingArc() {
    for (auto LUT : m_LUTs) {
      delete LUT;
    }
  }

  LUT *getLUTByType(LUTType type) {
    for (auto lut : m_LUTs) {
      if (lut->getLUTType() == type) {
        return lut;
      }
    }
    return nullptr;
  }
  // getters
  LibPin *getFromPin() { return m_from_pin; }

  TimingType getTimingType() { return m_timing_type; }

private:
  LibPin     *m_from_pin; // related_pin
  LibPin     *m_to_pin;
  TimingType  m_timing_type;
  TimingSense m_timing_sense;
  string      m_when;
  vector<LUT *> m_LUTs;
};

typedef struct InputTimingInfo_s {
  float capacitance_min[2]; // indexed by RFType
  float capacitance_max[2]; // indexed by RFType
} InputTimingInfo;

typedef struct OutputTimingInfo_s {
  vector<LibTimingArc *> timing_arcs;
  string function;
} OutputTimingInfo;


typedef union TimingInfo_u {
  InputTimingInfo *in_tinfo;
  OutputTimingInfo *out_tinfo;
} TimingInfo;

class LibPin {
public:
  LibPin(LibCell *parent) {
    m_parent = parent;
    m_type = NONE;
    m_t_info.in_tinfo = nullptr;
  }

  TimingInfo &initTimingInfo(DirectionType iotype) {
    assert(iotype != NONE && iotype != INOUT);

    if (m_type != NONE) {
      // clear if it exists
      clearTimingInfo();
    }

    // set direction
    m_type = iotype;

    // allocate InputTimingInfo type if pin is input
    if (iotype == INPUT) {

      m_t_info.in_tinfo =
          (InputTimingInfo *)calloc(1, sizeof(InputTimingInfo));
    } else { // likewise for OutputTimingInfo
      m_t_info.out_tinfo =
          (OutputTimingInfo *)calloc(1, sizeof(OutputTimingInfo));
    }

    return m_t_info;
  }

  TimingInfo *getTimingInfo() { return &m_t_info; }
  void clearOutputTimingInfo(OutputTimingInfo *o_tinfo) {
    if (o_tinfo == nullptr) {
      return;
    }
    for (auto arc : o_tinfo->timing_arcs) {
      delete arc;
    }
  }

  void clearTimingInfo() {
    // clear if it exists
    if (m_type == INPUT) {
      if (m_t_info.in_tinfo) {
        free(m_t_info.in_tinfo);
        m_t_info.in_tinfo = nullptr;
      }
    } else {
      clearOutputTimingInfo(m_t_info.out_tinfo);
      free(m_t_info.out_tinfo);
      m_t_info.out_tinfo = nullptr;
    }
  }
  const string &getName() { return *m_name; }

  void setName(const string *name) {m_name = name;}
  ~LibPin() {
    clearTimingInfo();
  }

  void write_lef_for_ASP(ofstream &lefFile) {
    lefFile << "PIN " << *m_name << endl;
    lefFile << "DIRECTION " << directionToString(m_type) << endl;
    lefFile << "END " << *m_name << endl << endl;
  }

  void printInputTimingInfo() {
    cout << "\t\trise_capacitance_range: "
         << m_t_info.in_tinfo->capacitance_min[RISE] << ", "
         << m_t_info.in_tinfo->capacitance_max[RISE] << endl;
    cout << "\t\tfall_capacitance_range: "
         << m_t_info.in_tinfo->capacitance_min[FALL] << ", "
         << m_t_info.in_tinfo->capacitance_max[FALL] << endl;
  }
  void printOutputTimingInfo() {
    cout << "\t\tfunction: " << m_t_info.out_tinfo->function << endl;
    for (auto arc : m_t_info.out_tinfo->timing_arcs) {
      arc->print();
    }
  }

  void print() {
    cout << "\tPin: " << *m_name << endl;
    cout << "\t\tdirection :" << directionToString(m_type) << endl;

    if (m_type == INPUT) {
      printInputTimingInfo();
    } else {
      printOutputTimingInfo();
    }
  }

  DirectionType getType() { return m_type; }

private:
  const string *m_name; // points to lib pin unordered_map inside lib cell class
  DirectionType m_type;
  TimingInfo m_t_info;
  LibCell *m_parent;
};

class LibCell {
public:
  LibCell(string name) {
    m_name = name;
    m_ff_group = nullptr;
  }
  ~LibCell() {
    for (auto entry : m_pins) {
      delete entry.second;
    }
    if (m_ff_group) {
      free(m_ff_group);
    }
  }
  void insertPin(LibPin *pin, string name) {
    if (pin) {
      auto iter = m_pins.insert({name, pin});
      pin->setName(&(iter.first->first));
    }
  }
  LibPin *getPin(string pin_name) {
    auto item = m_pins.find(pin_name);
    if (item == m_pins.end()) {
      return nullptr;
    } else {
      return item->second;
    }
  }

  string &getName() {
    return m_name;
  }
  void print() {
    cout << "Cell: " << m_name << endl;
    if (m_ff_group) {
      cout << " Cell Type: SEQUENTIAL" << endl;
      cout << "\tFF Group Info: " << endl;
      if (m_ff_group->clocked_on_pin) {
        cout << "\t\tClocked On Pin: " << m_ff_group->clocked_on_pin->getName() << endl;
      }
      if (m_ff_group->preset_pin) {
        cout << "\t\tPreset Pin: " << m_ff_group->preset_pin->getName() << endl;
      }
      if (m_ff_group->clear_pin) {
        cout << "\t\tClear Pin: " << m_ff_group->clear_pin->getName() << endl;
      }
    }
    else {
      cout << " Cell Type: COMBINATIONAL" << endl;
    }
    for (auto pin : m_pins) {
      pin.second->print();
    }
  }
  void write_lef_for_ASP(ofstream &lefFile) {
    lefFile << "MACRO " << m_name << endl;
    for (auto pin : m_pins) {
      pin.second->write_lef_for_ASP(lefFile);
    }
    lefFile << "END " << m_name << endl << endl;
  }

  map<string, LibPin *> &getPins() { return m_pins; }

  ff_group *m_ff_group;

private:
  string m_name;
  map<string, LibPin *> m_pins;
};


class Lib {
public:
  Lib(string name) {
    m_name = name;
  }

  void set_time_unit(float time_unit) { m_time_unit = time_unit; }
  void set_cap_unit(float cap_unit) { m_cap_unit = cap_unit; }
  void set_voltage_unit(float voltage_unit) { m_voltage_unit = voltage_unit; }
  float get_time_unit() { return m_time_unit; }
  float get_cap_unit() { return m_cap_unit; }
  float get_voltage_unit() { return m_voltage_unit; }
  void insertCell(LibCell *cell) {
    m_cells.insert({cell->getName(), cell});
  }
  void insertTemplate(string name, LUTTemplate *lut_template) {
    m_templates[name] = lut_template;
  }
  LibCell *getCellByName(string cell_name) {
    return m_cells[cell_name];
  }
  LUTTemplate *getTemplateByName(string template_name) {
    return m_templates[template_name];
  }
  void print() {
    cout << "Library: " << m_name << endl;
    cout << "Library Time Unit: " << m_time_unit << endl;
    cout << "Library Capacitance Unit: " << m_cap_unit << endl;
    for (auto templ : m_templates) {
      templ.second->print();
    }

    for (auto cell : m_cells) {
      cell.second->print();
    }
  }

  void write_lef_for_ASP(string filepath) {
    ofstream lefFile(filepath);
    for (auto cell : m_cells) {
      cell.second->write_lef_for_ASP(lefFile);
    }
  }

private:
  map<string, LUTTemplate *> m_templates;
  unordered_map<string_view, LibCell *> m_cells;
  string m_name;
  float m_time_unit;
  float m_cap_unit;
  float m_voltage_unit;
};
#endif
