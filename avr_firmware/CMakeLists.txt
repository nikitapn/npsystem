
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/avr_firmware.stamp
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/avr_firmware bash clean.sh
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/avr_firmware perl build_all.pl
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/avr_firmware.stamp
    DEPENDS
      src/core_c.c
      src/core_s.S
      src/crc.S
      src/msvc.h
      src/net_c.c
      src/net_s.S
      src/pc_link.c
      src/pclink/timers.c
      src/pclink/timers.h
      src/pc_link_v.c
      src/self_prog.S
      src/tables.S
      src/twi.c
      src/zero_memory.S
      src/cfg/config.h
      src/cfg/hconfig_m16.h
      src/cfg/hconfig_m8.h
      src/cfg/sconfig_m16.h
      src/cfg/sconfig_m8.h
      src/tcpip/enc28j60.c
      src/tcpip/enc28j60.h
      src/tcpip/lan.c
      src/tcpip/lan.h
)

add_custom_target(avr_firmware DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/avr_firmware.stamp)
