### TCL script to test the CADI tool functionalities ###

# set file paths #
set LIB_FILE test/ihp013_openPDK.lib
set VERILOG_FILE test/c17.v
set OUTPUT_NETLIST outputs/c17_out.v

# load liberty file and netlist #
load_liberty $LIB_FILE
load_verilog $VERILOG_FILE

# create structures on API #
create_data_structures

# run unit tests #
run_unit_test

# write verilog file #
# Uncomment the next line to write the output netlist when you finish the implementation of structures #
# write_verilog $OUTPUT_NETLIST

# exit tool #
exit
