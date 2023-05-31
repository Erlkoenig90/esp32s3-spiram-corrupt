# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.


idf_component_register(
    SRCS main.c
    PRIV_REQUIRES  driver esp_pm esp_hw_support # optional, list the private requirements
)
