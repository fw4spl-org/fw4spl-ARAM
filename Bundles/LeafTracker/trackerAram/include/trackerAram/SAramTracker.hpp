/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2009-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __TRACKERARAM_SARAMTRACKER_HPP__
#define __TRACKERARAM_SARAMTRACKER_HPP__


#include <ARAM/TagDetector.hpp> // Main ARAM class
#include <ARAM/tag/HammingTagMatcher.hpp> // Tag validator
#include <ARAM/ROIDetector/CannyFittingDetector.hpp> // Region of interest detection
#include <exception>
#include <Windows.h>
#include <ARAM/export.hpp>
#include <ARAM/tools/Intrinsic.hpp>

#include "trackerAram/config.hpp"
#include "tracker/ITracker.hpp"

#include <extData/FrameTL.hpp>

#include <fwCom/Slot.hpp>
#include <fwCom/Slots.hpp>

#include <fwData/Composite.hpp>
#include <fwData/Image.hpp>

#include <fwCore/HiResClock.hpp>

#include <fwServices/Base.hpp>

#include <boost/shared_ptr.hpp>

using namespace aram;

namespace trackerAram
{

/**
 * @class   SAramTracker
 * @brief   Class used to track multiple tags with aram.
 */

class TRACKERARAM_CLASS_API SAramTracker : public ::tracker::ITracker
{
public:


    fwCoreServiceClassDefinitionsMacro((SAramTracker)(tracker::ITracker));

    typedef ::fwCom::Signal< void (::fwCore::HiResClock::HiResClockType timestamp) > DetectionDoneSignalType;
    typedef ::fwCom::Signal< void () > TagDetectedSignalType;
    typedef ::fwCom::Signal< void () > NoTagDetectedSignalType;
    typedef ::fwCom::Signal< void () > AramNotStartedSignalType;

    /// Key in m_signals map of signal m_sigDetectionDone
    TRACKERARAM_API static const ::fwCom::Signals::SignalKeyType s_DETECTION_DONE_SIG;

    TRACKERARAM_API static const ::fwCom::Signals::SignalKeyType s_TAG_DETECTED_SIG;
    TRACKERARAM_API static const ::fwCom::Signals::SignalKeyType s_NOTAG_DETECTED_SIG;
    TRACKERARAM_API static const ::fwCom::Signals::SignalKeyType s_ARAM_NOT_STARTED_SIG;

    // TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_METHOD_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_BLOCKSIZE_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_CONSTANT_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_BORDERWIDTH_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_PATTERNWIDTH_SLOT;

    //TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_MATCHERMETHOD_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_DISPLAY_TAGS_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_CHANGE_SPEED_SLOT;

    TRACKERARAM_API static const ::fwCom::Slots::SlotKeyType s_DETECT_MARKER_SLOT;

    typedef ::fwCom::Slot<void (::fwCore::HiResClock::HiResClockType)> DetectMarkerSlotType;

    typedef ::fwCom::Slot< void (double) >             ChangeBlockSizeSlotType;
    typedef ::fwCom::Slot< void (double) >             ChangeConstantSlotType;
    typedef ::fwCom::Slot< void (double) >             ChangeBorderWidthSlotType;
    typedef ::fwCom::Slot< void (double) >             ChangePatternWidthSlotType;
    typedef ::fwCom::Slot< void (unsigned int) >       ChangeSpeedSlotType;
    typedef ::fwCom::Slot< void (bool) >               DisplayTagsSlotType;



    typedef std::map<int, std::string> MarkerMapType;

    typedef std::pair< std::string, std::vector< int > >   MarkerPairType;
    typedef std::vector< MarkerPairType >                  TimelineVectorType;

    typedef aram::TagDetector<aram::CannyFittingDetector, aram::HammingTagMatcher> CannyDetectorType;
    /**
     * @brief Constructor.
     */
    TRACKERARAM_API SAramTracker() throw ();

    /**
     * @brief Destructor.
     */
    TRACKERARAM_API virtual ~SAramTracker() throw ();

protected:
    /**
     * @brief Configuring method : This method is used to configure the service.
     *
     * The method verifies if the configuration is well written and retrieves user parameter values.
     *
     * @verbatim
        <service uid="..." impl="::arTracker::SAramTracker" autoConnect="no">
            <config>
                <timelineVideo>frameTL</timelineVideo>
                <camera>camera</camera>
                <imageKey>image</imageKey>
                <cameraSettings>camera_data.xml</cameraSettings>
                <debugMarkers>yes</debugMarkers>
            </config>
        </service>
       @endverbatim
     * With :
     * None of these parameters are mandatory.
     *  - \b timelineVideo (mandatory) : input data.
     *  - \b camera (mandatory) camera key
     *    \b imageKey (mandatory)
     *    \b cameraSettings : file containing the camera setting (susch as calibration .. )
     *  - \b debugMarkers : if value is yes markers debugging mode is activated.
     */
    TRACKERARAM_API void configuring() throw (fwTools::Failed);

    /**
     * @brief Starting method : This method is used to initialize the service.
     */
    TRACKERARAM_API void starting() throw (fwTools::Failed);

    /**
     * @brief Updating method : This method is used to update the service.
     */
    TRACKERARAM_API void updating() throw (fwTools::Failed);

    /**
     * @brief Stopping method : This method is used to stop the service.
     */
    TRACKERARAM_API void stopping() throw (fwTools::Failed);

    /// Detect marker slot
    void detectMarker(::fwCore::HiResClock::HiResClockType timestamp);

    /// Threshold method slot
    // void setMethod(unsigned int);
    /// 'Constant' parameters slot
    void setConstant(double);
    /// Block size slot
    void setBlockSize(double);
    /// BorderWidth slot (not used)
    void setBorderWidth(double);
    /// PatternWidth slot (not used)
    void setPatternWidth(double);
    /// Method for matcher method slot
    //void setMatcherMethod(unsigned int);
    /// Method that change speed of detection (the lower the better but also the slower)
    void setSpeed(unsigned int);
    /// Method that display tags
    void displayTags(bool);

private:

    void updateImage(const ::fwCore::HiResClock::HiResClockType& timestamp);
    void drawTag(const std::string id, const aram::vecPoint2D& imgPoint, ::cv::Mat& inImage, int& color);

private:

    /// Video timeline key
    std::string m_timelineVideoKey;

    /// Marker map ['id':'timeline_key']
    MarkerMapType m_markers;

    /// Video camera key
    std::string m_cameraKey;

    /// Frame timeline
    ::extData::FrameTL::sptr m_frameTL;

    /// Image key
    std::string m_imageKey;

    /// Image
    ::fwData::Image::sptr m_image;

    /// Last timestamp
    ::fwCore::HiResClock::HiResClockType m_lastTimestamp;

    /// True if tracker is initialized
    bool m_isInitialized;

    /// Slots used when the frame have been refreshed
    DetectMarkerSlotType::sptr m_slotDetectMarker;

    /// Connections
    ::fwServices::helper::SigSlotConnection::sptr m_connections;

    /// BlockSize of the pixel neighborhood that is used to calculate a threshold value for the pixel
    double m_blockSize;

    /// The constant subtracted from the mean or weighted mean
    double m_constant;

    /// Display markers in the image or not
    bool m_debugMarkers;

    /// camera settings file name
    std::string m_cameraSettingFilename;

    CannyDetectorType *m_detector;

    /// The timeline vector. containing a MarkerPairType object;
    TimelineVectorType m_timelineVector;

    /// Signal to emit when
    DetectionDoneSignalType::sptr m_sigDetectionDone;
    TagDetectedSignalType::sptr m_sigTagDetected;
    NoTagDetectedSignalType::sptr m_sigNoTagDetected;
    AramNotStartedSignalType::sptr m_sigAramNotStarted;

    ///Slot called when threshold method changed
    // ChangeMethodSlotType::sptr m_slotChangeMethod;
    ///Slot called when block size changed
    ChangeBlockSizeSlotType::sptr m_slotChangeBlockSize;
    ///Slot called when constant parameters changed
    ChangeConstantSlotType::sptr m_slotChangeConstant;
    ///Slot called when border width changed
    ChangeBorderWidthSlotType::sptr m_slotChangeBorderWidth;
    ///Slot called when pattern width changed
    ChangePatternWidthSlotType::sptr m_slotChangePatternWidth;
    ///Slot called when method for matcher method changed
    // ChangeMatcherMethodSlotType::sptr m_slotChangeMatcherMethod;
    ///Slot called when speed of detection changed
    ChangeSpeedSlotType::sptr m_slotChangeSpeed;
    ///Slot called when debug mode is enabled/disabled
    DisplayTagsSlotType::sptr m_slotDisplayTags;


};

} //namespace trackerAram

#endif /* __TRACKERARAM_SARAMTRACKER_HPP__ */
