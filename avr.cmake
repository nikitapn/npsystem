
add_subdirectory(avr_firmware)
add_subdirectory(make_info)

add_custom_command(
  OUTPUT avr_copy_firmware.stamp
  COMMENT "Copying firmware files"
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
      ${CMAKE_SOURCE_DIR}/avr_firmware/.out/firmware
      $<TARGET_FILE_DIR:npsystem>/data/avr/firmware
  DEPENDS avr_firmware
)

add_custom_command(
  OUTPUT generate_info.stamp
  COMMENT "Creating memory addresses file"
  COMMAND ${CMAKE_COMMAND} -E touch generate_info.stamp
  COMMAND make_info --out-dir $<TARGET_FILE_DIR:npsystem>/data
  DEPENDS avr_firmware make_info
)

add_custom_target(avr DEPENDS avr_copy_firmware.stamp generate_info.stamp)
