/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2009-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "trackerAram/SAramTracker.hpp"

#include <arData/Camera.hpp>
#include <arData/MarkerTL.hpp>

#include <fwCore/Profiling.hpp>

#include <fwCom/Signal.hxx>
#include <fwCom/Slots.hxx>

#include <fwcomEd/helper/Array.hpp>

#include <fwData/Composite.hpp>
#include <fwData/mt/ObjectWriteLock.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <exception>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

//-----------------------------------------------------------------------------

using namespace aram;
namespace trackerAram

{
fwServicesRegisterMacro(::tracker::ITracker, ::trackerAram::SAramTracker, ::fwData::Composite);
//-----------------------------------------------------------------------------

const ::fwCom::Signals::SignalKeyType SAramTracker::s_DETECTION_DONE_SIG   = "detectionDone";
const ::fwCom::Signals::SignalKeyType SAramTracker::s_TAG_DETECTED_SIG     = "tagDetected";
const ::fwCom::Signals::SignalKeyType SAramTracker::s_NOTAG_DETECTED_SIG   = "noTagDetected";
const ::fwCom::Signals::SignalKeyType SAramTracker::s_ARAM_NOT_STARTED_SIG = "aramNotStarted";

//const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_METHOD_SLOT           = "changeMethod";
const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_BLOCKSIZE_SLOT    = "changeBlockSize";
const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_CONSTANT_SLOT     = "changeConstant";
const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_BORDERWIDTH_SLOT  = "changeBorderWidth";
const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_PATTERNWIDTH_SLOT = "changePatternWidth";
//const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_MATCHERMETHOD_SLOT = "changeMatcherMethod";
const ::fwCom::Slots::SlotKeyType SAramTracker::s_CHANGE_SPEED_SLOT = "changeSpeed";
const ::fwCom::Slots::SlotKeyType SAramTracker::s_DISPLAY_TAGS_SLOT = "displayTags";

const ::fwCom::Slots::SlotKeyType SAramTracker::s_DETECT_MARKER_SLOT = "detectMarker";

//-----------------------------------------------------------------------------

SAramTracker::SAramTracker() throw () :
    m_lastTimestamp(0),
    m_isInitialized(false),
    m_blockSize(7.),
    m_constant(7.),
    m_debugMarkers(false),
    m_detector(nullptr)
{

    m_sigDetectionDone = DetectionDoneSignalType::New();
    m_signals( s_DETECTION_DONE_SIG,  m_sigDetectionDone);

    m_sigTagDetected = TagDetectedSignalType::New();
    m_signals( s_TAG_DETECTED_SIG,  m_sigTagDetected);

    m_sigNoTagDetected = NoTagDetectedSignalType::New();
    m_signals( s_NOTAG_DETECTED_SIG,  m_sigNoTagDetected);

    m_sigAramNotStarted = AramNotStartedSignalType::New();
    m_signals( s_ARAM_NOT_STARTED_SIG,  m_sigAramNotStarted);

#ifdef COM_LOG
    ::fwCom::HasSignals::m_signals.setID();
#endif

    m_slotChangeBlockSize = ::fwCom::newSlot(&SAramTracker::setBlockSize, this);
    m_slotChangeConstant  = ::fwCom::newSlot(&SAramTracker::setConstant, this);
    m_slotDisplayTags     = ::fwCom::newSlot(&SAramTracker::displayTags, this);
    m_slotDetectMarker    = ::fwCom::newSlot(&SAramTracker::detectMarker,this);

    ::fwCom::HasSlots::m_slots(s_CHANGE_BLOCKSIZE_SLOT, m_slotChangeBlockSize);
    ::fwCom::HasSlots::m_slots(s_CHANGE_CONSTANT_SLOT, m_slotChangeConstant);
    ::fwCom::HasSlots::m_slots(s_DETECT_MARKER_SLOT, m_slotDetectMarker);
    ::fwCom::HasSlots::m_slots(s_DISPLAY_TAGS_SLOT, m_slotDisplayTags);

    ::fwCom::HasSlots::m_slots.setWorker( m_associatedWorker );

    m_connections = ::fwServices::helper::SigSlotConnection::New();

}

//-----------------------------------------------------------------------------

SAramTracker::~SAramTracker() throw ()
{
}


//-----------------------------------------------------------------------------

void SAramTracker::configuring() throw (::fwTools::Failed)
{
    ::fwRuntime::ConfigurationElement::sptr cfg = m_configuration->findConfigurationElement("config");
    SLM_ASSERT("Tag 'config' not found.", cfg);

    // gets video timeline
    {
        ::fwRuntime::ConfigurationElement::sptr cfgTimelineVideo = cfg->findConfigurationElement("timelineVideo");
        SLM_ASSERT("Tag 'timelineVideo' not found.", cfgTimelineVideo);
        m_timelineVideoKey = cfgTimelineVideo->getValue();
        SLM_ASSERT("'timelineVideo' is empty", !m_timelineVideoKey.empty());
    }

    // gets marker informations
    {
        ::fwRuntime::ConfigurationElement::sptr cfgTrack = cfg->findConfigurationElement("track");
        SLM_ASSERT("Tag 'track' not found.", cfgTrack);
        typedef ::fwRuntime::ConfigurationElementContainer::Container CfgContainer;
        for(const CfgContainer::value_type& elt : cfgTrack->getElements())
        {
            SLM_ASSERT("Missing 'id' attribute.", elt->hasAttribute("id"));
            SLM_ASSERT("Missing 'timeline' attribute.", elt->hasAttribute("timeline"));

            const std::string currentTimeline = elt->getAttributeValue("timeline");
            const std::string markersID       = elt->getAttributeValue("id");

            m_timelineVector.push_back(MarkerPairType());
            MarkerPairType& markerPair = m_timelineVector.back();
            markerPair.first = currentTimeline;

            ::boost::tokenizer<> tok(markersID);
            for( ::boost::tokenizer<>::iterator it = tok.begin(); it!=tok.end(); ++it)
            {
                const int id = ::boost::lexical_cast< int >(*it);
                m_markers[id] = currentTimeline;
                markerPair.second.push_back(id);
            }
        }
    }

    // gets camera
    {
        ::fwRuntime::ConfigurationElement::sptr cfgCamera = cfg->findConfigurationElement("camera");
        SLM_ASSERT("Tag 'camera' not found.", cfgCamera);
        m_cameraKey = cfgCamera->getValue();
        SLM_ASSERT("'camera' is empty", !m_cameraKey.empty());
    }

    // get  the file containing the the camera information
    ::fwRuntime::ConfigurationElement::sptr cfgCameraSettings = cfg->findConfigurationElement("cameraSettings");
    SLM_ASSERT("Tag 'cameraSettings' not found.", cfgCameraSettings);
    m_cameraSettingFilename = cfgCameraSettings->getValue();
    SLM_ASSERT("'cameraSettings' is empty", !m_cameraSettingFilename.empty());

    // get the debug markers flag
    ::fwRuntime::ConfigurationElement::sptr cfgDebugMarkers = cfg->findConfigurationElement("debugMarkers");
    if (cfgDebugMarkers && cfgDebugMarkers->getValue() == "yes")
    {
        m_debugMarkers = true;
    }

    //image
    ::fwRuntime::ConfigurationElement::sptr cfgImage = cfg->findConfigurationElement("imageKey");
    SLM_ASSERT("The image key of the service ::trackerAram::SAramTracker is not specified "
               "(imageKey element is missing)", cfgImage);
    m_imageKey = cfgImage->getValue();

}

//-----------------------------------------------------------------------------

void SAramTracker::starting() throw (::fwTools::Failed)
{
    ::fwData::Composite::sptr comp = this->getObject< ::fwData::Composite >();

    ::extData::FrameTL::sptr m_frameTL = comp->at< ::extData::FrameTL >(m_timelineVideoKey);

    m_image = ::fwData::Image::dynamicCast((*comp)[m_imageKey]);
    OSLM_ASSERT("The image \"" << m_imageKey << "\" is not valid.", m_image);

    m_connections->connect(m_frameTL, ::extData::TimeLine::s_OBJECT_PUSHED_SIG, this->getSptr(),
                           ::trackerAram::SAramTracker::s_DETECT_MARKER_SLOT);

    OSLM_ASSERT("cameraSettings file  " << m_cameraSettingFilename.c_str() << " doesn't exist ",
                ::boost::filesystem::exists(m_cameraSettingFilename));

    // initialized marker timeline matrix (4*2D points)
    for(const MarkerPairType &elt : m_timelineVector)
    {
        ::arData::MarkerTL::sptr markerTL = comp->at< ::arData::MarkerTL >(elt.first);
        markerTL->initPoolSize(static_cast<unsigned int>(elt.second.size()));
    }

    // Tag detector instanciation
    m_detector = new CannyDetectorType();

    // Intrinsics parameters
    aram::Intrinsic intr(m_cameraSettingFilename);

    // Set the status to Not Strated.
    m_sigAramNotStarted->asyncEmit();
}

//-----------------------------------------------------------------------------

void SAramTracker::stopping() throw (::fwTools::Failed)
{
    m_connections->disconnect();
    delete m_detector;
    m_isInitialized = false;
}

//-----------------------------------------------------------------------------

void SAramTracker::updating() throw (::fwTools::Failed)
{
}

//-----------------------------------------------------------------------------

void SAramTracker::detectMarker(::fwCore::HiResClock::HiResClockType timestamp)
{
    //    ::fwData::mt::ObjectWriteLock destLock(m_image);
    if (timestamp > m_lastTimestamp)
    {
        ::fwData::Composite::sptr comp = this->getObject< ::fwData::Composite >();

        m_frameTL = comp->at< ::extData::FrameTL >(m_timelineVideoKey);
        if(!m_isInitialized)
        {
            //Image size
            ::cv::Size2i size(static_cast<int>(m_frameTL->getWidth()), static_cast<int>(m_frameTL->getHeight()));
            m_isInitialized = true;
        }

        ::fwCore::HiResClock::HiResClockType timestamp     = m_frameTL->getNewerTimestamp();
        const CSPTR(::extData::FrameTL::BufferType) buffer = m_frameTL->getClosestBuffer(timestamp);

        OSLM_WARN_IF("Buffer not found with timestamp "<< timestamp, !buffer );
        if(buffer)
        {
            m_lastTimestamp = timestamp;

            const ::boost::uint8_t* frameBuff = &buffer->getElement(0);

            // read the input image
            ::cv::Mat inImage =
                ::cv::Mat (::cv::Size(static_cast<int>(m_frameTL->getWidth()),
                                      static_cast<int>(m_frameTL->getHeight())), CV_8UC4, (void*)frameBuff,
                           ::cv::Mat::AUTO_STEP);
            {
                FW_PROFILE("ARAM DECTECTOR");
                m_detector->detect(inImage);
            }

            aram::iteratorTag it;

            // Put the tag info in a timeline
            SPTR(::arData::MarkerTL::BufferType) trackerObject;
            for (it = m_detector->begin(); it != m_detector->end(); ++it)
            {
                std::string idTag("id=");
                idTag += std::to_string( (*it)->id());
                std::string associatedTL = m_markers[(*it)->id()];

                for(const MarkerPairType &tlPair : m_timelineVector)
                {
                    for(unsigned int markerPosition = 0; markerPosition < tlPair.second.size(); ++markerPosition)
                    {
                        if(associatedTL == tlPair.first)
                        {
                            ::arData::MarkerTL::sptr markerTL = comp->at< ::arData::MarkerTL >(tlPair.first);
                            aram::vecPoint2D imgPoint = (*it)->corners();
                            float markerBuffer[8];
                            for (size_t j = 0; j < 4; ++j)
                            {
                                markerBuffer[2*j]   = imgPoint[j].x;
                                markerBuffer[2*j+1] = imgPoint[j].y;
                            }
                            if(trackerObject == NULL)
                            {
                                trackerObject = markerTL->createBuffer(timestamp);
                                markerTL->pushObject(trackerObject);
                            }
                            trackerObject->setElement(markerBuffer,  markerPosition);
                            if(m_debugMarkers)
                            {
                                ///for each marker, draw info and its boundaries in the image
                                int color(25); // Variable used to change the color of the different tags
                                cv::Point centroidTag(0,0);
                                this->drawTag(idTag, imgPoint, inImage, color);
                            }
                        }
                    }
                }
            }

            //            if(m_debugMarkers)
            //            {
            //                ///for each marker, draw info and its boundaries in the image
            //                int color(25); // Variable used to change the color of the different tags
            //
            //                // Loop over the tags, in order to draw the informations on the image
            //                for (it = m_detector->begin(); it != m_detector->end(); ++it)
            //                {
            //                    // For each tag, a vector is created, containing the corner of the tag
            //                    aram::vecPoint2D imgPoint = (*it)->corners();
            //
            //                    // Chain of characters which will be draw on the tag (tag id number)
            //                    std::string idTag("id=");
            //                    idTag += std::to_string( (*it)->id());
            //
            //                    cv::Point centroidTag(0,0);     // variable containing the coordinate of the centroid of the tag
            //
            //                    // Loop drawing point at each corner of the tag and calculation of the centroid
            //                    for (unsigned int i = 0; i<imgPoint.size(); ++i)
            //                    {
            //                        cv::circle(inImage, imgPoint[i], 2.5, cv::Scalar(250-2*color,12+color,30+2*color,200), 2.8);
            //
            //                        centroidTag.x += imgPoint[i].x;
            //                        centroidTag.y += imgPoint[i].y;
            //
            //                        cv::line(inImage,imgPoint[i%4],imgPoint[(i+1)%4],cv::Scalar(250-2*color,12+color,30+2*color,
            //                                                                                    200),1.7);
            //                    }
            //
            //                    centroidTag.x /= 4.;
            //                    centroidTag.y /= 4.;
            //
            //                    color += 20;
            //                    // Drawing the id of the tag
            //                    putText(inImage, idTag, centroidTag, cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(70,250,250,200), 2);
            //                }

            this->updateImage(timestamp);
        }

        // Notify tag detection or not.
        if((m_detector->tag())->size())
        {
            m_sigTagDetected->asyncEmit();
        }
        else
        {
            m_sigNoTagDetected->asyncEmit();
        }

        //Notify detection
        OSLM_INFO("Detection Done for" << (m_detector->tag())->size() << " Tag(s) for timestamp : " << timestamp );
        m_sigDetectionDone->asyncEmit(timestamp);
    }
}

//-----------------------------------------------------------------------------

void SAramTracker::drawTag(const std::string id, const aram::vecPoint2D& imgPoint, ::cv::Mat& inImage, int& color)
{
    cv::Point centroidTag(0,0);     // variable containing the coordinate of the centroid of the tag

    // Loop drawing point at each corner of the tag and calculation of the centroid
    for (unsigned int i = 0; i<imgPoint.size(); ++i)
    {
        cv::circle(inImage, imgPoint[i], 2.5, cv::Scalar(250-2*color,12+color,30+2*color,200), 2.8);

        centroidTag.x += imgPoint[i].x;
        centroidTag.y += imgPoint[i].y;

        cv::line(inImage,imgPoint[i%4],imgPoint[(i+1)%4],cv::Scalar(250-2*color,12+color,30+2*color,
                                                                    200),1.7);
    }

    centroidTag.x /= 4.;
    centroidTag.y /= 4.;

    color += 20;
    // Drawing the id of the tag
    putText(inImage, id, centroidTag, cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(70,250,250,200), 2);
}

//-----------------------------------------------------------------------------

void SAramTracker::updateImage(const ::fwCore::HiResClock::HiResClockType& timestamp)
{
    ::fwData::Array::sptr array = m_image->getDataArray();

    ::fwComEd::helper::Array arrayHelper(array);

    CSPTR(::extData::FrameTL::BufferType) buffer = m_frameTL->getClosestBuffer(timestamp);

    OSLM_WARN_IF("Buffer not found with timestamp "<< timestamp, !buffer );
    if(buffer)
    {

        const ::boost::uint8_t* frameBuff = &buffer->getElement(0);

        ::extData::timeline::Buffer::BufferDataType index = arrayHelper.begin< ::boost::uint8_t >();

        std::copy( frameBuff, frameBuff+buffer->getSize(), index);

        ::fwData::Image::BufferModifiedSignalType::sptr sig;
        sig = m_image->signal< ::fwData::Image::BufferModifiedSignalType >( ::fwData::Image::s_BUFFER_MODIFIED_SIG );
        {
            ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
            sig->asyncEmit();
        }
    }
}

//-----------------------------------------------------------------------------

void SAramTracker::setBlockSize(double size)
{
    m_blockSize = size;
}

//-----------------------------------------------------------------------------

void SAramTracker::setConstant(double constant)
{
    m_constant = constant;
}

//-----------------------------------------------------------------------------

void SAramTracker::displayTags(bool b)
{
    m_debugMarkers = b;
}

} // namespace trackerAram
