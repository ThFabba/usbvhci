
add_definitions(-DNTDDI_VERSION=0x05020400)

list(APPEND SOURCE
    businterface.c
    fdo.c
    pdo.c
    power.c
    usbvhci.c
    wmi.c
    usbvhci.h)

add_library(usbvhci SHARED
    ${SOURCE}
    guid.c)
    #usbvhci.rc)

target_link_libraries(usbvhci uuid memcmp)
set_module_type(usbvhci kernelmodedriver)
add_importlibs(usbvhci ntoskrnl hal)
add_pch(usbvhci usbvhci.h SOURCE)
add_cd_file(TARGET usbvhci DESTINATION reactos/system32/drivers NO_CAB FOR all)
