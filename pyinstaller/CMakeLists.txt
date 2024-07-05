cmake_minimum_required(VERSION 3.17)

#add_pyinstall_target(std_stream_binary_jf_sim SOURCE std_stream_binary_jf_sim.py)
#add_pyinstall_target(std_udp_sim_gf SOURCE std_udp_sim_gf.py)

add_pyinstall_target(fake_trigger_sender
    SOURCE fake_trigger_sender.py
    HIDDEN_IMPORTS std_buffer
)

add_pyinstall_target(rest_primary
    SOURCE rest_primary.py
    HIDDEN_IMPORTS std_buffer
)

add_pyinstall_target(rest_secondary
    SOURCE rest_secondary.py
    HIDDEN_IMPORTS std_buffer
)