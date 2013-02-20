# Include directory (same as the source... bad bad bad)
include_directories(.)

# Executable
add_executable(predictor predictor.cc op_state.cc cbp_inst.cc tread.cc main.cc)

