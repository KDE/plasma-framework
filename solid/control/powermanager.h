/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_CONTROL_POWERMANAGER_H
#define SOLID_CONTROL_POWERMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QHash>

#include "solid_control_export.h"

class KJob;

namespace Solid
{
namespace Control
{
    /**
     * This namespace allows to query the underlying system to obtain information
     * about the hardware available.
     *
     * It's the unique entry point for power management. Applications should use
     * it to control or query the power management features of the system.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    namespace PowerManager
    {
       /**
         * This enum type defines the different states of the system battery.
         *
         * - NoBatteryState: No battery available
         * - Normal: The battery is at its normal charge level
         * - Warning: The battery is at its warning charge level
         * - Low: The battery is at its low charge level
         * - Critical: The battery is at its critical charge level
         */
        enum BatteryState{ NoBatteryState, Normal, Warning, Low, Critical };

        /**
         * This enum type defines the different states of the AC adapter.
         *
         * - UnknownAcAdapterState: The AC adapter has an unknown state
         * - Plugged: The AC adapter is plugged
         * - Unplugged: The AC adapter is unplugged
         */
        enum AcAdapterState{ UnknownAcAdapterState, Plugged, Unplugged };

        /**
         * This enum type defines the types of system button events.
         *
         * - UnknownButtonType: An unknown button
         * - PowerButton: A power button pressed event, generally used to turn on or off the system
         * - SleepButton: A sleep button pressed event, generally used to make the system asleep
         * - LidOpen: A laptop lid open event
         * - LidClose: A laptop lid close event
         */
        enum ButtonType{ UnknownButtonType, PowerButton, SleepButton, LidOpen, LidClose };

        /**
         * This enum type defines the different suspend methods.
         *
         * - UnknownSuspendMethod: The name says it all
         * - Standby: Processes are stopped, some hardware is deactivated (ACPI S1)
         * - ToRam: Most devices are deactivated, only RAM is powered (ACPI S3)
         * - ToDisk: State of the machine is saved to disk, and it's powered down (ACPI S4)
         */
        enum SuspendMethod{ UnknownSuspendMethod = 0, Standby = 1, ToRam = 2, ToDisk = 4};

        /**
         * This type stores an OR combination of SuspendMethod values.
         */
        Q_DECLARE_FLAGS(SuspendMethods, SuspendMethod)

        /**
         * This enum type defines the different CPU frequency policies.
         *
         * - UnknownCpuFreqPolicy: The name says it all
         * - OnDemand: Frequency is changed by the kernel depending on the processor load
         * - Conservative: Frequency is changed by the kernel depending on the processor load; the stepping is less aggressive than OnDemand.  This may be equivalent to OnDemand depending on the operating system.
         * - Userspace: Frequency is changed by a userspace agent depending on the processor load
         * - Powersave: Frequency is always set to the lowest available
         * - Performance: Frequency is always set to the highest available
         */
        enum CpuFreqPolicy{ UnknownCpuFreqPolicy = 0, OnDemand = 1, Userspace = 2, Powersave = 4, Performance = 8, Conservative = 16 };

        /**
         * This type stores an OR combination of CpuFreqPolicy values.
         */
        Q_DECLARE_FLAGS(CpuFreqPolicies, CpuFreqPolicy)

        /**
         * This enum defines the different types of brightness controls.
         *
         * - UnknownBrightnessControl: Unknown
         * - Screen: Brightness control for a monitor or laptop panel
         * - Keyboard: Brightness control for a keyboard backlight
         */
        enum BrightnessControlType{ UnknownBrightnessControl = 0, Screen = 1, Keyboard = 2 };

        typedef QHash<QString, BrightnessControlType> BrightnessControlsList;


        /**
         * Retrieves the list of power management schemes available on this system.
         *
         * @return the available power management schemes
         */
        SOLIDCONTROL_EXPORT QStringList supportedSchemes();

        /**
         * Retrieves a localized description corresponding to the given scheme.
         *
         * @param schemeName the name of the scheme we request the description for
         * @return the associated description
         */
        SOLIDCONTROL_EXPORT QString schemeDescription(const QString &schemeName);

        /**
         * Retrieves the name of the current power management scheme used
         * by the system.
         *
         * @return the current scheme
         */
        SOLIDCONTROL_EXPORT QString scheme();

        /**
         * Changes the current power management scheme.
         *
         * @param name the name of the new scheme
         * @return true if the scheme change succeeded, false otherwise
         */
        SOLIDCONTROL_EXPORT bool setScheme(const QString &name);


        /**
         * Retrieves the current state of the system battery.
         *
         * @return the current battery state
         * @see Solid::Control::PowerManager::BatteryState
         */
        SOLIDCONTROL_EXPORT BatteryState batteryState();

        /**
         * Retrieves the current charge percentage of the system batteries.
         *
         * @return the current global battery charge percentage
         */
        SOLIDCONTROL_EXPORT int batteryChargePercent();

        /**
         * Retrieves the current state of the system AC adapter.
         *
         * @return the current AC adapter state
         * @see Solid::Control::PowerManager::AcAdapterState
         */
        SOLIDCONTROL_EXPORT AcAdapterState acAdapterState();


        /**
         * Retrieves the set of suspend methods supported by the system.
         *
         * @return the suspend methods supported by this system
         * @see Solid::Control::PowerManager::SuspendMethod
         * @see Solid::Control::PowerManager::SuspendMethods
         */
        SOLIDCONTROL_EXPORT SuspendMethods supportedSuspendMethods();

        /**
         * Requests a suspend of the system.
         *
         * @param method the suspend method to use
         * @return the job handling the operation
         */
        SOLIDCONTROL_EXPORT KJob *suspend(SuspendMethod method);


        /**
         * Retrieves the set of CPU frequency policies supported by the system.
         *
         * @return the CPU frequency policies supported by this system
         * @see Solid::Control::PowerManager::CpuFreqPolicy
         * @see Solid::Control::PowerManager::CpuFreqPolicies
         */
        SOLIDCONTROL_EXPORT CpuFreqPolicies supportedCpuFreqPolicies();

        /**
         * Retrieves the current CPU frequency policy of the system.
         *
         * @return the current CPU frequency policy used by the system
         * @see Solid::Control::PowerManager::CpuFreqPolicy
         */
        SOLIDCONTROL_EXPORT CpuFreqPolicy cpuFreqPolicy();

        /**
         * Changes the current CPU frequency policy of the system.
         *
         * @param newPolicy the new policy
         * @return true if the policy change succeeded, false otherwise
         * @see Solid::Control::PowerManager::CpuFreqPolicy
         */
        SOLIDCONTROL_EXPORT bool setCpuFreqPolicy(CpuFreqPolicy newPolicy);

        /**
         * Checks if a CPU can be disabled.
         *
         * @param cpuNum the number of the CPU we want to check
         * @return true if the given CPU can be disabled, false otherwise
         */
        SOLIDCONTROL_EXPORT bool canDisableCpu(int cpuNum);

        /**
         * Enables or disables a CPU.
         *
         * @param cpuNum the number of the CPU we want to enable or disable
         * @param enabled the new state of the CPU
         * @return true if the state change succeeded, false otherwise
         */
        SOLIDCONTROL_EXPORT bool setCpuEnabled(int cpuNum, bool enabled);

        /**
         * Checks if brightness controls are enabled on this system.
         *
         * @return a list of the devices available to control
         */
        SOLIDCONTROL_EXPORT BrightnessControlsList brightnessControlsAvailable();

        /**
         * Gets the screen brightness.
         *
         * @param device the name of the device that you would like to control
         * @return the brightness of the device, as a percentage
         */
        SOLIDCONTROL_EXPORT float brightness(const QString &device = QString());

        /**
         * Sets the screen brightness.
         *
         * @param brightness the desired screen brightness, as a percentage
         * @param device the name of the device that you would like to control
         * @return true if the brightness change succeeded, false otherwise
         */
        SOLIDCONTROL_EXPORT bool setBrightness(float brightness, const QString &device = QString());

        class SOLIDCONTROL_EXPORT Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
            /**
             * This signal is emitted when the power management scheme has changed.
             *
             * @param newScheme the new scheme name
             */
            void schemeChanged(QString newScheme);

            /**
             * This signal is emitted when the AC adapter is plugged or unplugged.
             *
             * @param newState the new state of the AC adapter, it's one of the
             * type @see Solid::Control::PowerManager::AcAdapterState
             */
            void acAdapterStateChanged(int newState);

            /**
             * This signal is emitted when the system battery state changed.
             *
             * @param newState the new state of the system battery, it's one of the
             * type @see Solid::Control::PowerManager::BatteryState
             */
            void batteryStateChanged(int newState);

            /**
             * This signal is emitted when a button has been pressed.
             *
             * @param buttonType the pressed button type, it's one of the
             * type @see Solid::Control::PowerManager::ButtonType
             */
            void buttonPressed(int buttonType);

            /**
             * This signal is emitted when the brightness changes.
             *
             * @param brightness the new brightness level
             */
             void brightnessChanged(float brightness);
        };

        SOLIDCONTROL_EXPORT Notifier *notifier();
    }
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::PowerManager::SuspendMethods)
Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::PowerManager::CpuFreqPolicies)

#endif
