include_directories(${PROJECT_SOURCE_DIR}/include/minidocx)

link_libraries(minidocx)

add_executable(myapp main.cpp)
add_executable(pict picture.cpp)
add_executable(table table.cpp)
add_executable(style style.cpp)
add_executable(list list.cpp)
add_executable(inspection_workflow inspection_workflow.cpp)

set_target_properties(
  myapp
  pict
  table
  style
  list
  inspection_workflow
  PROPERTIES FOLDER examples)
