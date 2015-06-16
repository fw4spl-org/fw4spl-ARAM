link_directories(${CMAKE_PREFIX_PATH}/lib)

fwLoadProperties()
fwUseForwardInclude(
    fwRuntime
    fwCore
    fwData
    fwDataTools
    fwServices
    fwCom
    fwComEd
    fwTools
    fwMath
    arData
    extData
)

find_package(OpenCV REQUIRED) 
find_package(aram REQUIRED)
link_libraries(${EXTERNAL_LIBRARIES}/lib/aram.lib)

fwLink(
    ${OpenCV_LIBS}
    ${EXTERNAL_LIBRARIES}/lib/aram.lib
    
)

