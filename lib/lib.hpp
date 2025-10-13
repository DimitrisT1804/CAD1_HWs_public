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
// Authors: Stavros Simoglou
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
typedef enum {INPUT_SLEW = 0, OUTPUT_LOAD = 1} LUTVariableType;
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
class Lib;
class LibCell;
class LibArc;
class LibPin;
class LibTimingArc;

class LUT {
public:
  LUT(LUTType type, LUTDataType *data) {
    m_type = type;
    m_data = data;
  }

  ~LUT() {
    if (m_data) {
      liberty_destroy_value_data(m_data);
    }
  }
  LUTDataType *getLUTData() {
    return m_data;
  }
private:
  LUTType      m_type;
  LUTDataType *m_data;
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

  ~LibTimingArc() {
    for (auto LUT : m_LUTs) {
      delete LUT;
    }
  }
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
  vector<LibTimingArc> timing_arcs;
  string function;
} OutputTimingInfo;

typedef union TimingInfo_u {
  InputTimingInfo *in_tinfo;
  OutputTimingInfo *out_tinfo;
  TimingInfo_u() {
    in_tinfo = nullptr;
    out_tinfo = nullptr;
  }

  ~TimingInfo_u() {
    if (in_tinfo) {
      delete in_tinfo;
    }
    if (out_tinfo) {
      delete out_tinfo;
    }
  }
} TimingInfo;

class LibPin {
public:
  LibPin(LibCell *parent) {
    m_parent = parent;
    m_type = NONE;
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
          (InputTimingInfo *)calloc(1, sizeof(InputTimingInfo_s));
    } else { // likewise for OutputTimingInfo
      m_t_info.out_tinfo =
          (OutputTimingInfo *)calloc(1, sizeof(OutputTimingInfo_s));
    }
    return m_t_info;
  }

  TimingInfo &getTimingInfo() { return m_t_info; }

  void clearTimingInfo() {
    // clear if it exists
    if (m_type == INPUT) {
      if (m_t_info.in_tinfo) {
        delete m_t_info.in_tinfo;
      }
    } else {
      if (m_t_info.out_tinfo) {
        delete m_t_info.out_tinfo;
      }
        delete m_t_info.out_tinfo;
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

  void print() { cout << "\tPin: " << *m_name << endl; }

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
  }
  ~LibCell() {
    for (auto entry : m_pins) {
      delete entry.second;
    }
  }
  void insertPin(LibPin *pin, string name) {
    if (pin) {
      auto iter = m_pins.insert({name, pin});
      pin->setName(&(iter.first->first));
    }
  }
  LibPin *getPin(string pin_name) {
    return m_pins[pin_name];
  }

  string &getName() {
    return m_name;
  }
  void print() {
    cout << "Cell: " << m_name << endl;
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

private:
  string m_name;
  map<string, LibPin *> m_pins;
};


class LUTTemplate {
public:
  LUTTemplate() {

  }
  string &getName() {
    return m_name;
  }

  void print() {
    cout << "Template: " << m_name << endl;
  }
private:
  LUTVariableType m_variable1;
  LUTVariableType m_variable2;
  vector<float> m_index1;
  vector<float> m_index2;
  string m_name;
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
  void insertTemplate(LUTTemplate *lut_template) {
    m_templates.push_back(lut_template);
  }
  LibCell *getCellByName(string &cell_name) {
    return m_cells[cell_name];
  }
  int getTemplateIndexByName(string &template_name) {
    for (auto tmpl = m_templates.begin(); tmpl != m_templates.end(); ++tmpl) {
      if ((*tmpl)->getName() == template_name) {
        return (int) distance(m_templates.begin(), tmpl);
      }
    }
    return -1;
  }
  LUTTemplate *getTemplateByName(string &template_name) {
    int index = getTemplateIndexByName(template_name);
    if (index != -1) {
      return m_templates[index];
    } else {
      return nullptr;
    }
  }
  void print() {
    cout << "Library: " << m_name << endl;
    cout << "Library Time Unit: " << m_time_unit << endl;
    cout << "Library Capacitance Unit: " << m_cap_unit << endl;
    for (auto templ : m_templates) {
      templ->print();
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
  vector<LUTTemplate *> m_templates;
  unordered_map<string_view, LibCell *> m_cells;
  string m_name;
  float m_time_unit;
  float m_cap_unit;
  float m_voltage_unit;
};
#endif
