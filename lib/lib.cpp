//   ____    _    ____    ___    ____                            ____            _           _
//  / ___|  / \  |  _ \  |_ _|  / ___|___  _   _ _ __ ___  ___  |  _ \ _ __ ___ (_) ___  ___| |_
// | |     / _ \ | | | |  | |  | |   / _ \| | | | '__/ __|/ _ \ | |_) | '__/ _ \| |/ _ \/ __| __|
// | |___ / ___ \| |_| |  | |  | |__| (_) | |_| | |  \__ \  __/ |  __/| | | (_) | |  __/ (__| |_
//  \____/_/   \_\____/  |___|  \____\___/ \__,_|_|  |___/\___| |_|   |_|  \___// |\___|\___|\__|
//   
// A project by University of Thessaly CASlab and Stavros Simoglou, for the purpose of academic
// training on the topics of physical design algorithms, EDA CAD, and larger project development
//
// File:    lib.cpp
// Purpose: library parser
// Authors: Stavros Simoglou
//

#include <lib.hpp>
#include <cstring>

using namespace std;

int parse_liberty(string libfile_path, Lib **library)
{
    si2drErrorT err;
    si2drGroupsIdT groups;
    si2drGroupIdT group;

    time_t startt, endt;

    char *filename = (char *) libfile_path.c_str();
    cout << "Reading Liberty..." << endl;
    si2drPIInit(&err);
    si2drReadLibertyFile(filename, &err);

    time(&startt);
    if( err == SI2DR_INVALID_NAME )
    {
      cout << endl << filename << " Does Not Exist." << endl;
        return(301);
    }
    else if (err == SI2DR_SYNTAX_ERROR )
    {
      cout << endl << "Syntax Error!" << endl << endl;
        return(401);
    }

    time(&endt);

    cout << endl << "... Done. \nElapsed time= %ld seconds" << endl;


    // database check
    groups = si2drPIGetGroups(&err);
    while( !si2drObjectIsNull( (group=si2drIterNextGroup(groups,&err)), &err))  {
      cout << endl << "Checking the database..." << endl;

        time(&startt);
        si2drCheckLibertyLibrary(group, &err);
        time(&endt);

        if( err == SI2DR_NO_ERROR )
          cout << "Passed" << endl << endl;
        else {
            cout << "Errors detected during Liberty sanity check" << endl << endl;;
            return(501);
        }

        cout << "... Done. \nElapsed time= %ld seconds" << endl << endl;

    }
    si2drIterQuit(groups,&err);


    // no syntax errors; build library
    si2drGroupsIdT library_groups = si2drPIGetGroups(&err);
    si2drGroupIdT library_group;

    // iterate through library group
    while( !si2drObjectIsNull((library_group=si2drIterNextGroup(library_groups, &err)), &err) )  {

        //si2drStringT library_group_type = si2drGroupGetGroupType(library_group, &err);
        //printf(" confirm: library type = %s\n", library_group_type);


        // library name
        si2drNamesIdT library_group_names = si2drGroupGetNames(library_group, &err);
        si2drStringT library_group_name = si2drIterNextName(library_group_names, &err);
        si2drIterQuit(library_group_names, &err);

        // create library
        (*library) = new Lib(string(library_group_name));

        // process library level attributes
        si2drAttrsIdT library_attrs = si2drGroupGetAttrs(library_group, &err);
        si2drAttrIdT library_attr;
        while( !si2drObjectIsNull((library_attr=si2drIterNextAttr(library_attrs, &err)), &err)) {
            std::string library_attr_name = si2drAttrGetName(library_attr, &err);

            float unit = 0.0;
            if (library_attr_name == "time_unit") {
              (*library)->set_time_unit(unit);
            } else if (library_attr_name == "voltage_unit") {
              unit = stof(si2drSimpleAttrGetStringValue(library_attr, &err));
              (*library)->set_voltage_unit(unit);
            } else if (library_attr_name == "capacitive_load_unit") {

              si2drValuesIdT cap_values =
                  si2drComplexAttrGetValues(library_attr, &err);

              // storage location for result
              si2drValueTypeT type;
              si2drInt32T int_val;
              si2drFloat64T double_val;
              si2drStringT string_val;
              si2drBooleanT bool_val;
              si2drExprT *expr;

              si2drIterNextComplexValue(cap_values, &type, &int_val,
                                        &double_val, &string_val, &bool_val,
                                        &expr, &err);
              float unit_value = int_val;

              si2drIterNextComplexValue(cap_values, &type, &int_val,
                                        &double_val, &string_val, &bool_val,
                                        &expr, &err);
              std::string unit_name = string_val;
              si2drIterQuit(cap_values, &err);

              if (unit_name == "ff") {
                unit_value *= 1e-15;
              } else /* default to pf */ {
                unit_value *= 1e-12;
              }
              (*library)->set_cap_unit(unit_value);
            }


        }
        si2drIterQuit(library_attrs, &err);

        // get lib cells
        si2drGroupsIdT cell_groups = si2drGroupGetGroups(library_group, &err);
        si2drGroupIdT cell_group;
        while( !si2drObjectIsNull((cell_group=si2drIterNextGroup(cell_groups, &err)), &err)) {
            si2drStringT cell_group_type =  si2drGroupGetGroupType(cell_group, &err);

            // skip non "cell" group types
            if (strcmp(cell_group_type, "cell")) {
                continue;
            }

            // get name of lib cell
            si2drNamesIdT cell_group_names = si2drGroupGetNames(cell_group, &err);
            string cell_group_name = string((char *) si2drIterNextName(cell_group_names, &err));
            si2drIterQuit(cell_group_names, &err);

            // insert lib cell into library
            LibCell *cell = new LibCell(cell_group_name);
            (*library)->insertCell(cell);

            // first insert all pins. Needed for figuring out related_pin
            // pointer when parsing timing arcs
            si2drGroupsIdT pin_groups = si2drGroupGetGroups(cell_group, &err);
            si2drGroupIdT pin_group;
            while( !si2drObjectIsNull((pin_group=si2drIterNextGroup(pin_groups, &err)), &err)) {

                si2drStringT pin_group_type = si2drGroupGetGroupType(pin_group, &err);
                //printf("   confirm: pin type = %s\n", pin_group_type);

                // skip if not a "pin"
                // i.e. we don't process pg_pin, leakage_power, etc.
                if (strcmp(pin_group_type,"pin")) {
                    continue;
                }

                // get lib pin name
                si2drNamesIdT pin_group_names = si2drGroupGetNames(pin_group, &err);
                si2drStringT pin_group_name = si2drIterNextName(pin_group_names, &err);
                si2drIterQuit(pin_group_names, &err);

                // create lib pin
                LibPin *pin = new LibPin(cell);

                // add lib pin to lib cell
                cell->insertPin(pin, string(pin_group_name));

            }
            si2drIterQuit(pin_groups, &err);

            // now process lib pins
            pin_groups = si2drGroupGetGroups(cell_group, &err);
            while( !si2drObjectIsNull((pin_group=si2drIterNextGroup(pin_groups, &err)), &err)) {

                si2drStringT pin_group_type = si2drGroupGetGroupType(pin_group, &err);
                //printf("   confirm: pin type = %s\n", pin_group_type);

                // skip if not a "pin"
                // i.e. we don't process pg_pin, leakage_power, etc.
                if (strcmp(pin_group_type,"pin")) {
                    continue;
                }

                // get lib pin name
                si2drNamesIdT pin_group_names = si2drGroupGetNames(pin_group, &err);
                si2drStringT pin_group_name = si2drIterNextName(pin_group_names, &err);
                si2drIterQuit(pin_group_names, &err);

                // get lib pin from cell
                LibPin *pin = cell->getPin(string(pin_group_name));

                // look for direction attribute
                si2drAttrsIdT pin_attrs = si2drGroupGetAttrs(pin_group, &err);
                si2drAttrIdT pin_attr;
                float rise_cap = 0.0, fall_cap = 0.0;
                while (!si2drObjectIsNull(
                    (pin_attr = si2drIterNextAttr(pin_attrs, &err)), &err)) {
                  std::string pin_attr_name = si2drAttrGetName(pin_attr, &err);

                  // look for direction; for the purposes of the course
                  // direction must be defined before all other pin info
                  DirectionType direction = NONE;
                  if (pin_attr_name == "direction") {
                    std::string direction_str =
                        si2drSimpleAttrGetStringValue(pin_attr, &err);

                    if (direction_str == "input") {
                      direction = INPUT;
                    } else if (direction_str == "output") {
                      direction = OUTPUT;
                    } else { // skip inouts
                      break;
                    }

                    pin->initTimingInfo(direction);
                    continue;
                  }

                  TimingInfo &t_info = pin->getTimingInfo();

                  // look for rise_capacitance. Set it to both min and max if
                  // they are not set by rise_capacitance_range
                  if (pin_attr_name == "rise_capacitance") {
                    rise_cap = si2drSimpleAttrGetFloat64Value(pin_attr, &err);
                    float &t_info_r_cap_min =
                        t_info.in_tinfo->capacitance_min[RISE];
                    float &t_info_r_cap_max =
                        t_info.in_tinfo->capacitance_max[RISE];
                    if (t_info_r_cap_min == 0.0) {
                      t_info_r_cap_min = rise_cap;
                    }
                    if (t_info_r_cap_max == 0.0) {
                      t_info_r_cap_max = rise_cap;
                    }
                  }

                  // look for fall_capacitance. Set it to both min and max if
                  // they are not set by fall_capacitance_range
                  if (pin_attr_name == "fall_capacitance") {
                    fall_cap = si2drSimpleAttrGetFloat64Value(pin_attr, &err);
                    float &t_info_f_cap_min =
                        t_info.in_tinfo->capacitance_min[FALL];
                    float &t_info_f_cap_max =
                        t_info.in_tinfo->capacitance_max[FALL];
                    if (t_info_f_cap_min == 0.0) {
                      t_info_f_cap_min = fall_cap;
                    }
                    if (t_info_f_cap_max == 0.0) {
                      t_info_f_cap_max = fall_cap;
                    }
                  }

                  // look for rise_capacitance_range
                  if (pin_attr_name == "rise_capacitance_range") {
                    si2drValuesIdT cap_values =
                        si2drComplexAttrGetValues(pin_attr, &err);

                    // storage location for result
                    si2drValueTypeT type;
                    si2drInt32T int_val;
                    si2drFloat64T double_val;
                    si2drStringT string_val;
                    si2drBooleanT bool_val;
                    si2drExprT *expr;

                    // now iterate through the cap_values, grabbing first two
                    // floats
                    si2drIterNextComplexValue(cap_values, &type, &int_val,
                                              &double_val, &string_val,
                                              &bool_val, &expr, &err);
                    float rise_cap_min = double_val;
                    t_info.in_tinfo->capacitance_min[RISE] = rise_cap_min;
                    si2drIterNextComplexValue(cap_values, &type, &int_val,
                                              &double_val, &string_val,
                                              &bool_val, &expr, &err);
                    float rise_cap_max = double_val;
                    t_info.in_tinfo->capacitance_max[RISE] = rise_cap_max;
                    si2drIterQuit(cap_values, &err);
                  }

                  // look for rise_capacitance_range
                  if (pin_attr_name == "fall_capacitance_range") {
                    si2drValuesIdT cap_values =
                        si2drComplexAttrGetValues(pin_attr, &err);

                    // storage location for result
                    si2drValueTypeT type;
                    si2drInt32T int_val;
                    si2drFloat64T double_val;
                    si2drStringT string_val;
                    si2drBooleanT bool_val;
                    si2drExprT *expr;

                    // now iterate through the cap_values, grabbing first two
                    // floats
                    si2drIterNextComplexValue(cap_values, &type, &int_val,
                                              &double_val, &string_val,
                                              &bool_val, &expr, &err);
                    float fall_cap_min = double_val;
                    t_info.in_tinfo->capacitance_min[FALL] = fall_cap_min;
                    si2drIterNextComplexValue(cap_values, &type, &int_val,
                                              &double_val, &string_val,
                                              &bool_val, &expr, &err);
                    float fall_cap_max = double_val;
                    t_info.in_tinfo->capacitance_max[FALL] = fall_cap_max;
                    si2drIterQuit(cap_values, &err);
                  }
                }
                si2drIterQuit(pin_attrs, &err);

                // get lib arcs. they are defined as "timing ()" group in the lib file
                si2drGroupsIdT arc_groups = si2drGroupGetGroups(pin_group, &err);
                si2drGroupIdT arc_group;
                while( !si2drObjectIsNull((arc_group=si2drIterNextGroup(arc_groups, &err)), &err)) {

                    si2drStringT arc_group_type = si2drGroupGetGroupType(arc_group, &err);
                    //printf("     confirm: arc type = %s\n", arc_group_type);

                    // get name of group (template)
                    //si2drNamesIdT arc_group_names = si2drGroupGetNames(arc_group, &err);
                    //si2drStringT arc_group_name = si2drIterNextName(arc_group_names, &err);
                    //si2drIterQuit(arc_group_names, &err);
                    //printf("     confirm: arc template name = %s\n", arc_group_name);

                    // skip non timing arc groups
                    if (strcmp(arc_group_type, "timing")) {
                        continue;
                    }

                    // define arc variable for use

                    // process all timing group attributes
                    std::string related_pin_str;
                    // default timing_type is "combinational"
                    std::string timing_type_str("combinational");
                    std::string timing_sense_str;

                    si2drAttrsIdT attrs = si2drGroupGetAttrs(arc_group, &err);
                    si2drAttrIdT attr;

                    while( !si2drObjectIsNull((attr=si2drIterNextAttr(attrs, &err)), &err)) {
                        std::string name = si2drAttrGetName(attr, &err);

                        // look for related_pin
                        if (name == "related_pin") {
                          related_pin_str =
                              si2drSimpleAttrGetStringValue(attr, &err);
                        } else if (name == "timing_type") { // look timing_type
                          timing_type_str =
                              si2drSimpleAttrGetStringValue(attr, &err);
                        } else if (name == "timing_sense") { // look for timing_sense
                            timing_sense_str = si2drSimpleAttrGetStringValue(attr, &err);
                        }
                    }
                    si2drIterQuit(attrs, &err);

                    assert(!related_pin_str.empty() &&
                           "Undefined arc related pin ");

                    LibTimingArc *arc =
                        new LibTimingArc(cell->getPin(related_pin_str), pin);

                    arc->insertTimingType(stringToTimingType(timing_type_str));
                    arc->insertTimingSense(
                        stringToTimingSense(timing_sense_str));

                    // parse arc LUTs
                    si2drGroupsIdT table_groups = si2drGroupGetGroups(arc_group, &err);
                    si2drGroupIdT table_group;

                    LUTDataType *data;
                    LUT *lut;
                    LUTType type;
                    while (!si2drObjectIsNull(
                        (table_group = si2drIterNextGroup(table_groups, &err)),
                        &err)) {

                      si2drStringT table_group_type =
                          si2drGroupGetGroupType(table_group, &err);

                      data = nullptr;
                      // store NLDM tables
                      if (!strcmp(table_group_type, "cell_rise")) {
                        data = liberty_get_values_data(table_group);
                        type = CELL_RISE;
                      } else if (!strcmp(table_group_type, "cell_fall")) {
                        data = liberty_get_values_data(table_group);
                        type = CELL_FALL;
                      } else if (!strcmp(table_group_type, "rise_transition")) {
                        data = liberty_get_values_data(table_group);
                        type = RISE_TRANSITION;
                      } else if (!strcmp(table_group_type, "fall_transition")) {
                        data = liberty_get_values_data(table_group);
                        type = FALL_TRANSITION;
                      }

                      // store NLDM table if found
                      if (data) {
                        lut = new LUT(type, data);
                        arc->insertLUT(lut);
                      }
                    }
                    si2drIterQuit(table_groups, &err);
                }
                si2drIterQuit(arc_groups, &err);
            }
            si2drIterQuit(pin_groups, &err);

            // fix up arcs to have pointers to pins, for consistency
            // cell->fixup_pins_in_arcs();


        }
        si2drIterQuit(cell_groups, &err);

    }
    si2drIterQuit(library_groups, &err);

    si2drPIQuit(&err);
    printf("Done....\n");

    return(0);

}

//   return 1;
// }
