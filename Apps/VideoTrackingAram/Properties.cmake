
set( NAME VideoTrackingAram )
set( VERSION 0.1 )
set( TYPE APP )
set( UNIQUE TRUE)
set( DEPENDENCIES  )
set( REQUIREMENTS
    dataReg
    arDataReg
    extDataReg
    gui
    guiQt
    servicesReg
    visu
    launcher
    appXml
    visuVTK
    visuVTKQt
    visuVTKAdaptor
    visuVTKARAdaptor
    videoQt
    videoTools
    tracker
    trackerAram
    arMedia
    uiTrackingAramQt
    uiTools
    )

bundle_param(appXml PARAM_LIST config PARAM_VALUES VideoTrackingAram)

set(START_BUNDLES
    extDataReg
    arDataReg
    uiTrackingAramQt
    visuVTK
    visuVTKQt
    guiQt
    arMedia
    appXml

)