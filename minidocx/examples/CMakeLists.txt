include_directories(${PROJECT_SOURCE_DIR}/include/minidocx)

link_libraries(minidocx)

add_executable(myapp main.cpp)
add_executable(pict picture.cpp)
add_executable(table table.cpp)
add_executable(style style.cpp)
add_executable(list list.cpp)
add_executable(inspection_workflow inspection_workflow.cpp)
add_executable(pyproviders python_providers.cpp)
add_executable(pyproviders_docx python_providers_docx_companions.cpp)
add_executable(pyproviders_xml python_providers_xml_expert.cpp)
add_executable(pyproviders_media_pdf python_providers_media_pdf.cpp)

set_target_properties(
  myapp
  pict
  table
  style
  list
  inspection_workflow
  pyproviders
  pyproviders_docx
  pyproviders_xml
  pyproviders_media_pdf
  PROPERTIES FOLDER examples)
