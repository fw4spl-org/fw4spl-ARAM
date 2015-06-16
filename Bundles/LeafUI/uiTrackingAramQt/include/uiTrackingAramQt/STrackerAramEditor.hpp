/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2009-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __UITRACKINGARAMQT_STRACKERARAMEDITOR_HPP__
#define __UITRACKINGARAMQT_STRACKERARAMEDITOR_HPP__

#include "uiTrackingAramQt/config.hpp"

#include <fwCom/Signal.hpp>
#include <fwCom/Signals.hpp>

#include <fwData/Object.hpp>

#include <gui/editor/IEditor.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>

namespace uiTrackingAramQt
{
/**
 * @brief   STrackerAramEditor service used to handle parameters of aram tracker.
 * @class   STrackerAramEditor
 *
 * Service registered details : \n
 * fwServicesRegisterMacro(::gui::editor::IEditor, ::STrackerAramEditor::STrackerAramEditor, ::fwData::Object)
 */
class UITRACKINGARAMQT_CLASS_API STrackerAramEditor :
    public QObject,
    public ::gui::editor::IEditor
{
Q_OBJECT;

public:
    fwCoreServiceClassDefinitionsMacro((STrackerAramEditor)(::gui::editor::IEditor));

    /**
     * @name Signals API
     * @{
     */
    // typedef ::fwCom::Signal< void (unsigned int) > MethodChangedSignalType;
    // typedef ::fwCom::Signal< void (unsigned int) > MatcherMethodChangedSignalType;
    typedef ::fwCom::Signal< void (double) >       ConstantChangedSignalType;
    typedef ::fwCom::Signal< void (double) >       BlockSizeChangedSignalType;
    typedef ::fwCom::Signal< void (double) >       BorderWidthChangedSignalType;
    typedef ::fwCom::Signal< void (double) >       PatternWidthChangedSignalType;
    typedef ::fwCom::Signal< void (bool) >         TagsDisplayedSignalType;
    typedef ::fwCom::Signal< void (unsigned int) > SpeedChangedSignalType;

    /// Key in m_signals map of signal m_sigMethodChanged
    // UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_METHOD_CHANGED_SIG;

    /// Key in m_signals map of signal m_sigMatcherMethodChanged
    //UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_MATCHERMETHOD_CHANGED_SIG;

    /// Key in m_signals map of signal m_sigConstantChanged
    UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_CONSTANT_CHANGED_SIG;

    /// Key in m_signals map of signal m_sigBlockSizeChanged
    UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_BLOCKSIZE_CHANGED_SIG;

    /// Key in m_signals map of signal m_sigBorderWidthChanged
    UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_BORDERWIDTH_CHANGED_SIG;

    /// Key in m_signals map of signal m_sigPatternWidthChanged
    UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_PATTERNWIDTH_CHANGED_SIG;

    /// Key in m_signals map of signal m_sigDebugEnabled
    UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_TAGS_DISPLAYED_SIG;

    /// Key in m_signals map of signal m_sigSpeedChanged
    UITRACKINGARAMQT_API static const ::fwCom::Signals::SignalKeyType s_SPEED_CHANGED_SIG;

    ///@}


    /**
     * @brief Constructor.
     */
    UITRACKINGARAMQT_API STrackerAramEditor() throw ();

    /**
     * @brief Destructor.
     */
    UITRACKINGARAMQT_API ~STrackerAramEditor() throw ();

    ///nothing
    UITRACKINGARAMQT_API void configuring() throw (fwTools::Failed);

    /**
     * @brief Starting method : This method is used to initialize the service.
     */
    UITRACKINGARAMQT_API void starting() throw (fwTools::Failed);

    /**
     * @brief Stopping method : This method is used to stop the service.
     */
    UITRACKINGARAMQT_API void stopping() throw (fwTools::Failed);

    /**
     * @brief Updating method : This method is used to update the service.
     */
    UITRACKINGARAMQT_API void updating() throw (fwTools::Failed);

protected:

    /**
     * @brief Add a new point acquisition.
     * @param[in] capture Composite containing the point acquisitions from the two cameras.
     */
    UITRACKINGARAMQT_API void update();

protected Q_SLOTS:

    /// Slot called when Method comboBox value changed.
    //void onMethodChanged(int value);

    /// Slot called when Method comboBox value changed.
    void onConstantChanged(double value);

    /// Slot called when Method comboBox value changed.
    void onBlockSizeChanged(double value);

    /// Slot called when Method comboBox value changed.
    void onBorderWidthChanged(double value);

    /// Slot called when Method comboBox value changed.
    void onPatternWidthChanged(double value);

    /// Slot called when Method comboBox value changed.
    //void onMatcherMethodChanged(int value);

    ///Slot called when debug checkbox is enabled/disabled
    void onTagsDisplayed(int value);

    ///Slot called when speed comboBox changed
    void onSpeedChanged(int value);

private:

    QComboBox*      m_methodBox;
    QComboBox*      m_matcherBox;
    QComboBox*      m_speedBox;
    QDoubleSpinBox* m_constantSB;
    QDoubleSpinBox* m_blockSizeSB;
    QDoubleSpinBox* m_borderWidthSB;
    QDoubleSpinBox* m_patternWidthSB;
    QCheckBox*      m_displayTags;

    ConstantChangedSignalType::sptr m_sigConstantChanged;
    BlockSizeChangedSignalType::sptr m_sigBlockSizeChanged;
    BorderWidthChangedSignalType::sptr m_sigBorderWidthChanged;
    PatternWidthChangedSignalType::sptr m_sigPatternWidthChanged;
    TagsDisplayedSignalType::sptr m_sigTagsDisplayed;
    SpeedChangedSignalType::sptr m_sigSpeedChanged;

};
} // namespace uiTrackerAramQt

#endif // __UITRACKINGARAMQT_STRACKERARAMEDITOR_HPP__
