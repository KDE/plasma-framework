/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

// The PageStack item defines a container for pages and a stack-based
// navigation model. Pages can be defined as QML items or components.

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

import "." 2.0 as PlasmaComponents
import "private/PageStack.js" as Engine

/**
 * The PageStack component provides a stack-based navigation model that you can
 * use in your application. A stack-based navigation model means that a page of
 * content for your application is pushed onto a stack when the user navigates
 * deeper into the application page hierarchy. The user can then go back to the
 * previous page (or several pages back) by popping a page (or several pages)
 * off the top of the stack.
 *
 * @inherit QtQuick.Item
 */
Item {
    id: root

    width: parent ? parent.width : 0
    height: parent ? parent.height : 0

    /** The number of pages on the page stack. */
    property int depth: Engine.getDepth()

    /** The page in the stack that is currently visible. */
    property Item currentPage: null

    /**
     * The toolbar container for the tools associated with each page.
     *
     * If a toolbar is specified, the tools set for the current page is shown
     * to the user.
     *
     * If toolbar is null, then no tools are shown even if a page does have
     * tools.
     */
    property Item toolBar

    /**
     * Should page transitions be animated? Default is true.
     */
    property bool animate: PlasmaCore.Units.longDuration > 1

    /**
     * The page to be automatically loaded when this PageStack component gets
     * instantiated.
     */
    property var initialPage

    /** Indicates whether there is an ongoing page transition. */
    property bool busy: internal.ongoingTransitionCount > 0

    /**
     * Pushes a page on the stack.
     *
     * The page can be defined as a component, item or string.  If an item is
     * used then the page will get re-parented.  If a string is used then it is
     * interpreted as a url that is used to load a page component.
     *
     * The page can also be given as an array of pages. In this case all those
     * pages will be pushed onto the stack. The items in the stack can be
     * components, items or strings just like for single pages. Additionally an
     * object can be used, which specifies a page and an optional properties
     * property. This can be used to push multiple pages while still giving
     * each of them properties.  When an array is used the transition animation
     * will only be to the last page.
     *
     * Note: When the stack is empty, a push() or replace() does not perform a
     * transition animation because there is no page to transition from. The
     * only time this normally happens is when an application is starting up so
     * it is not appropriate to have a transition animation anyway.
     *
     * @param type:var page the page to push
     * @param type:dict properties optional argument to define a map of
     * properties to set on the page.
     * @param type:bool immediate set to true to disable transition animations
     *
     * @return type:Page the page instance.
     */
    function push(page, properties, immediate)
    {
        return Engine.push(page, properties, false, !root.animate || immediate);
    }

    /**
     * Pops a page off the stack.
     * If the page used to have another parent before being pushed, it will be reparented
     * to this old parent. If the page was created from a component at push time,
     * it will be deleted.
     *
     * @param type:Page page if page is specified then the stack is unwound to that
     * page, to unwind to the first page specify page as null.
     * @param type:bool immediate set to true to disable transition animations
     *
     * @return type:Page the page instance that was popped off the stack.
     */
    function pop(page, immediate)
    {
        return Engine.pop(page, !root.animate || immediate);
    }

    /**
     * Replaces the top-most page on the stack with page.
     *
     * As in the push() operation, you can use a component, item or string for
     * the page, or even an array of pages. If the page is based on the Item
     * element, the page is re-parented. As in the pop() operation, the
     * replaced page on the stack is re-parented back to its original parent.
     *
     * @param type:var page the page to push
     * @param type:dict properties optional argument to define a map of
     * properties to set on the page.
     * @param type:bool immediate set to true to disable transition animations
     *
     * @return type:Page The new top page on the stack.
     */
    function replace(page, properties, immediate)
    {
        return Engine.push(page, properties, true, !root.animate || immediate);
    }

    /** Clears the page stack of all pages. */
    function clear()
    {
        return Engine.clear();
    }

    /**
     * Iterates through all pages (top to bottom) and invokes the specified function.
     * If the specified function returns true the search stops and the find function
     * returns the page that the iteration stopped at. If the search doesn't result
     * in any page being found then null is returned.
     *
     * @param type:function func the function to call
     * @return type:Page the found page or null
     */
    function find(func)
    {
        return Engine.find(func);
    }

    implicitWidth: currentPage ? currentPage.implicitWidth : 0
    implicitHeight: currentPage ? currentPage.implicitHeight : 0

    // Called when the page stack visibility changes.
    onVisibleChanged: {
        if (currentPage) {
            internal.setPageStatus(currentPage, visible ? PlasmaComponents.PageStatus.Active : PlasmaComponents.PageStatus.Inactive);
            if (visible)
                currentPage.visible = currentPage.parent.visible = true;
        }
    }

    onInitialPageChanged: {
        if (!internal.completed) {
            return
        }

        if (initialPage) {
            if (depth === 0) {
                push(initialPage, null, true)
            } else if (depth === 1) {
                replace(initialPage, null, true)
            } else {
                console.log("Cannot update PageStack.initialPage")
            }
        }
    }

    Component.onCompleted: {
        internal.completed = true
        if (initialPage && depth === 0)
            push(initialPage, null, true)
    }

    QtObject {
        id: internal

        // The number of ongoing transitions.
        property int ongoingTransitionCount: 0

        //FIXME: there should be a way to access to them without storing it in an ugly way
        property bool completed: false

        // Sets the page status.
        function setPageStatus(page, status)
        {
            if (page !== null) {
                if (page.status !== undefined) {
                    if (status === PlasmaComponents.PageStatus.Active && page.status === PlasmaComponents.PageStatus.Inactive)
                        page.status = PlasmaComponents.PageStatus.Activating;
                    else if (status === PlasmaComponents.PageStatus.Inactive && page.status === PlasmaComponents.PageStatus.Active)
                        page.status = PlasmaComponents.PageStatus.Deactivating;

                    page.status = status;
                }
            }
        }
    }

    // Component for page containers.
    Component {
        id: containerComponent

        Item {
            id: container

            width: parent ? parent.width : 0
            height: parent ? parent.height : 0

            // The actual parent of page: page will anchor to that
            // in this case is container itself, not the case for PageRow
            property Item pageParent: container

            // The states correspond to the different possible positions of the container.
            state: "Hidden"

            // The page held by this container.
            property Item page: null

            // The owner of the page.
            property Item owner: null

            // The width of the longer stack dimension
            property int stackWidth: Math.max(root.width, root.height)

            // Duration of transition animation (in ms)
            property int transitionDuration: Math.max(1, PlasmaCore.Units.longDuration / 2)

            // Flag that indicates the container should be cleaned up after the transition has ended.
            property bool cleanupAfterTransition: false

            // Flag that indicates if page transition animation is running
            property bool transitionAnimationRunning: false

            // State to be set after previous state change animation has finished
            property string pendingState: "none"

            // Ensures that transition finish actions are executed
            // in case the object is destroyed before reaching the
            // end state of an ongoing transition
            Component.onDestruction: {
                if (transitionAnimationRunning)
                    transitionEnded();
            }

            // Sets pending state as current if state change is delayed
            onTransitionAnimationRunningChanged: {
                if (!transitionAnimationRunning && pendingState !== "none") {
                    state = pendingState;
                    pendingState = "none";
                }
            }

            // Handles state change depending on transition animation status
            function setState(newState)
            {
                if (transitionAnimationRunning)
                    pendingState = newState;
                else
                    state = newState;
            }

            // Performs a push enter transition.
            function pushEnter(immediate, orientationChanges)
            {
                if (!immediate) {
                    if (orientationChanges)
                        setState("LandscapeRight");
                    else
                        setState("Right");
                }
                setState("");
                page.visible = true;
                if (root.visible && immediate)
                    internal.setPageStatus(page, PlasmaComponents.PageStatus.Active);
            }

            // Performs a push exit transition.
            function pushExit(replace, immediate, orientationChanges)
            {
                if (orientationChanges)
                    setState(immediate ? "Hidden" : "LandscapeLeft");
                else
                    setState(immediate ? "Hidden" : "Left");
                if (root.visible && immediate)
                    internal.setPageStatus(page, PlasmaComponents.PageStatus.Inactive);
                if (replace) {
                    if (immediate)
                        cleanup();
                    else
                        cleanupAfterTransition = true;
                }
            }

            // Performs a pop enter transition.
            function popEnter(immediate, orientationChanges)
            {
                if (!immediate)
                    state = orientationChanges ? "LandscapeLeft" : "Left";
                setState("");
                page.visible = true;
                if (root.visible && immediate)
                    internal.setPageStatus(page, PlasmaComponents.PageStatus.Active);
            }

            // Performs a pop exit transition.
            function popExit(immediate, orientationChanges)
            {
                if (orientationChanges)
                    setState(immediate ? "Hidden" : "LandscapeRight");
                else
                    setState(immediate ? "Hidden" : "Right");

                if (root.visible && immediate)
                    internal.setPageStatus(page, PlasmaComponents.PageStatus.Inactive);
                if (immediate)
                    cleanup();
                else
                    cleanupAfterTransition = true;
            }

            // Called when a transition has started.
            function transitionStarted()
            {
                transitionAnimationRunning = true;
                internal.ongoingTransitionCount++;
                if (root.visible) {
                    internal.setPageStatus(page, (state === "") ? PlasmaComponents.PageStatus.Activating : PlasmaComponents.PageStatus.Deactivating);
                }
            }

            // Called when a transition has ended.
            function transitionEnded()
            {
                if (state !== "")
                    state = "Hidden";
                if (root.visible)
                    internal.setPageStatus(page, (state === "") ? PlasmaComponents.PageStatus.Active : PlasmaComponents.PageStatus.Inactive);

                internal.ongoingTransitionCount--;
                transitionAnimationRunning = false;
                if (cleanupAfterTransition)
                    cleanup();
            }

            states: [
                // Explicit properties for default state.
                State {
                    name: ""
                    PropertyChanges { target: container; visible: true; opacity: 1 }
                },
                // Start state for pop entry, end state for push exit.
                State {
                    name: "Left"
                    PropertyChanges { target: container; x: -width / 2; opacity: 0 }
                },
                // Start state for pop entry, end state for push exit
                // when exiting portrait and entering landscape.
                State {
                    name: "LandscapeLeft"
                    PropertyChanges { target: container; x: -stackWidth / 2; opacity: 0 }
                },
                // Start state for push entry, end state for pop exit.
                State {
                    name: "Right"
                    PropertyChanges { target: container; x: width / 2; opacity: 0 }
                },
                // Start state for push entry, end state for pop exit
                // when exiting portrait and entering landscape.
                State {
                    name: "LandscapeRight"
                    PropertyChanges { target: container; x: stackWidth / 2; opacity: 0 }
                },
                // Inactive state.
                State {
                    name: "Hidden"
                    PropertyChanges { target: container; visible: false }
                }
            ]

            transitions: [
                // Push exit transition
                Transition {
                    from: ""; to: "Left"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.InQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop entry transition
                Transition {
                    from: "Left"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.OutQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Push exit transition landscape
                Transition {
                    from: ""; to: "LandscapeLeft"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.InQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop entry transition landscape
                Transition {
                    from: "LandscapeLeft"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.OutQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop exit transition
                Transition {
                    from: ""; to: "Right"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.InQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        // Workaround for transition animation bug causing ghost view with page pop transition animation
                        // TODO: Root cause still unknown
                        PropertyAnimation {}
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Push entry transition
                Transition {
                    from: "Right"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.OutQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop exit transition landscape
                Transition {
                    from: ""; to: "LandscapeRight"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.InQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        // Workaround for transition animation bug causing ghost view with page pop transition animation
                        // TODO: Root cause still unknown
                        PropertyAnimation {}
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Push entry transition landscape
                Transition {
                    from: "LandscapeRight"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            XAnimator {
                                easing.type: Easing.OutQuad
                                duration: transitionDuration
                            }
                            OpacityAnimator {
                                easing.type: Easing.Linear
                                duration: transitionDuration
                            }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                }
            ]

            // Cleans up the container and then destroys it.
            function cleanup()
            {
                if (page !== null) {
                    if (page.status === PlasmaComponents.PageStatus.Active) {
                        internal.setPageStatus(page, PlasmaComponents.PageStatus.Inactive)
                    }
                    if (owner !== container) {
                        // container is not the owner of the page - re-parent back to original owner
                        page.visible = false;
                        page.anchors.fill = undefined
                        page.parent = owner;
                    }
                }

                container.destroy();
            }
        }
    }
}

