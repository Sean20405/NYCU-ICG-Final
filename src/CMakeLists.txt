add_executable(ICG_2024_NYCU-ICG-Final
"main.cpp"
"stb_image.cpp"
"shader.cpp"
"object.cpp"
) #列所有的cpp
target_link_libraries(ICG_2024_NYCU-ICG-Final
glfw
glm::glm
glad
)