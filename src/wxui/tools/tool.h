// Copyright 2026 Sergei Pikin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once
#include <map>
#include "../menucmd.h"

/** @brief SVG icon for enter/confirm action in the UI */
inline constexpr char enter_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16"><path fill="currentColor" d="m0 9l7 4v-3h9V3l-3 2v2H7V4z"/></svg>)rawsvg";

/** @brief SVG icon for step back/undo action in the UI */
inline constexpr char step_back_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16"><path fill="currentColor" d="M5.854 2.146a.5.5 0 0 1 0 .708L3.707 5h2.336c1.468 0 2.905 0 4.226.396c1.365.41 2.585 1.234 3.647 2.827a.5.5 0 0 1-.832.554c-.938-1.407-1.968-2.083-3.103-2.423C8.815 6.004 7.517 6 6 6H3.707l2.147 2.146a.5.5 0 1 1-.708.708l-3-3a.5.5 0 0 1 0-.708l3-3a.5.5 0 0 1 .708 0M8 14a2 2 0 1 0 0-4a2 2 0 0 0 0 4m0-1a1 1 0 1 1 0-2a1 1 0 0 1 0 2"/></svg>)rawsvg";

/** @brief SVG icon for cancel/abort action in the UI */
inline constexpr char cancel_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 48 48"><path fill="#D50000" d="M24 6C14.1 6 6 14.1 6 24s8.1 18 18 18s18-8.1 18-18S33.9 6 24 6m0 4c3.1 0 6 1.1 8.4 2.8L12.8 32.4C11.1 30 10 27.1 10 24c0-7.7 6.3-14 14-14m0 28c-3.1 0-6-1.1-8.4-2.8l19.6-19.6C36.9 18 38 20.9 38 24c0 7.7-6.3 14-14 14"/></svg>)rawsvg";

/** @brief SVG icon for finish/complete action in the UI */
inline constexpr char finish_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="1024" height="1024" viewBox="0 0 1024 1024"><path fill="currentColor" d="M280.8 753.7L691.5 167a32 32 0 1 1 52.4 36.7L314.2 817.5a32 32 0 0 1-45.4 7.3L38.4 652a32 32 0 0 1 38.4-51.2zM736 448a32 32 0 1 1 0-64h192a32 32 0 1 1 0 64zM608 640a32 32 0 0 1 0-64h320a32 32 0 1 1 0 64zM480 832a32 32 0 1 1 0-64h448a32 32 0 1 1 0 64z"/></svg>)rawsvg";

/** @brief SVG icon for vertex/point editing in the UI */
inline constexpr char vertexedit_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="cursor-edit-01"><title>cursor-edit-01</title><path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="1.5" d="m10.734 18.706l-2.281-6.554h0c-.858-2.465-1.287-3.697-.642-4.341s1.878-.216 4.345.641l6.545 2.275c1.369.476 2.053.713 2.2 1.174q.062.192.041.394c-.048.48-.668.855-1.908 1.604c-.796.48-1.193.72-1.3 1.082a1 1 0 0 0-.037.316c.017.376.346.704 1.005 1.358l2.164 2.151h0c.324.322.486.483.56.664c.098.235.098.5.002.736c-.074.18-.236.342-.559.665h0c-.322.322-.483.483-.664.557a.97.97 0 0 1-.735 0c-.18-.074-.341-.235-.664-.557h0L16.634 18.7c-.648-.648-.972-.971-1.344-.99a1 1 0 0 0-.333.04c-.356.108-.592.5-1.065 1.285h0c-.74 1.226-1.109 1.839-1.583 1.893a1 1 0 0 1-.415-.044c-.454-.15-.689-.826-1.16-2.178M4.5 2.5h-1a1 1 0 0 0-1 1v1a1 1 0 0 0 1 1h1a1 1 0 0 0 1-1v-1a1 1 0 0 0-1-1m0 11h-1a1 1 0 0 0-1 1v1a1 1 0 0 0 1 1h1a1 1 0 0 0 1-1v-1a1 1 0 0 0-1-1m11-11h-1a1 1 0 0 0-1 1v1a1 1 0 0 0 1 1h1a1 1 0 0 0 1-1v-1a1 1 0 0 0-1-1m-2 1.5h-8M4 5.5v8"></path></symbol>)rawsvg";

/**
 * @struct UICommand
 * @brief Represents a menu command with associated metadata.
 *
 * UICommand extends gceMenuCommand with a command identifier for use
 * in menu structures and command dispatch systems.
 */
struct UICommand : public gceMenuCommand
{
    /**
     * @brief Constructs a UI command.
     *
     * @param cmd The command identifier for this menu command.
     * @param label The display label for this command in the UI.
     */
    UICommand(int cmd, const wxString &label) : gceMenuCommand(label), m_cmd(cmd)
    {}

    /** @brief The unique command identifier. */
    int m_cmd = 0;
};

/**
 * @class UICommandContainer
 * @brief Container and manager for UI menu commands.
 *
 * UICommandContainer provides a convenient interface for building command lists,
 * generating menu structures, and managing keyboard accelerators for tools.
 */
class UICommandContainer
{
public:
    /**
     * @brief Adds or retrieves a UICommand with the specified ID and label.
     *
     * @param cmd The command identifier.
     * @param label The display label (optional, defaults to empty string).
     * @return A reference to the created or existing UICommand.
     */
    UICommand &operator ()(int cmd, const wxString &label = wxString())
    {
        return m_commands.emplace_back(cmd, label);
    }

    /**
     * @brief Adds a menu separator to the command list.
     */
    void separtor()
    {
        operator()(wxID_SEPARATOR);
    }

    /**
     * @brief Creates and displays a popup context menu from the commands.
     *
     * Generates a wxMenu from the accumulated commands and displays it
     * as a context menu relative to the specified window.
     *
     * @param win The window to display the popup menu relative to.
     * @param name Optional name/identifier for the menu.
     */
    void popupMenu(wxWindow *win, const wxString &name);

    /**
     * @brief Creates accelerator table entries for keyboard shortcuts.
     *
     * Generates a list of wxAcceleratorEntry objects from the commands
     * for keyboard shortcut handling. Results are cached.
     *
     * @return A const reference to the accelerator entry vector.
     */
    const std::vector<wxAcceleratorEntry> &createAccelerators2();

private:
    /** @brief Cached accelerator entries for keyboard shortcuts. */
    std::vector<wxAcceleratorEntry> m_accCache;

    /** @brief Container of UI commands in this menu structure. */
    std::vector<UICommand> m_commands;
};

class gceCanvas;

namespace geom
{
class CoordSeq;
class Geometry;
}
class gceToolOptions;
class gceEditorFrame;

struct umodelSelectXDResultMsg;
struct udataSelectReplyMsg;
struct udataMultiRowActionNotifyMsg;

/**
 * @struct gceToolInfo
 * @brief Metadata and display information for a tool.
 *
 * gceToolInfo contains static information about a tool including its name,
 * icon representation, and help text.
 */
struct gceToolInfo
{
    /** @brief Human-readable name of the tool. */
    const char *name;

    /** @brief SVG icon data string for the tool. */
    const char *svgIcon;

    /** @brief Help text describing the tool's functionality. */
    const char *help;
};

/**
 * @class gceToolBase
 * @brief Abstract base class for interactive tools in the geometry editor.
 *
 * gceToolBase defines the interface and common functionality for all interactive tools
 * used in the geometry editor. Tools handle user input, manage coordinate constraints,
 * control rendering, and provide UI options panels. Tools can be hierarchically organized
 * with parent-child relationships for nested tool operations.
 *
 * The tool lifecycle consists of:
 * - BeginUse(): Activate the tool
 * - DoRecalc() and Display(): Interactive operations
 * - EndUse(): Deactivate the tool
 *
 * @note This class is derived from wxEvtHandler for event processing.
 */
class gceToolBase : public wxEvtHandler
{
public:
    /**
     * @brief Constructs a tool instance.
     *
     * @param owner Reference to the editor frame that owns this tool.
     */
    explicit gceToolBase(gceEditorFrame &owner);

    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~gceToolBase();

    /**
     * @brief Checks if this tool is available in the current context.
     *
     * @return true if the tool can be used, false otherwise.
     */
    virtual bool isAvailable();

    /**
     * @brief Retrieves metadata and display information for this tool.
     *
     * @return A gceToolInfo structure containing tool name, icon, and help text.
     */
    virtual gceToolInfo GetInfo() const;

    /**
     * @brief Activates this tool for use, optionally with a parent tool.
     *
     * Initializes the tool and marks it as in-use. If a parent tool is provided,
     * this tool becomes a child of that parent for hierarchical tool management.
     *
     * @param parentTool The parent tool, if this is a child tool (nullptr if not).
     */
    void BeginUse(gceToolBase *parentTool);

    /**
     * @brief Deactivates and cleans up this tool.
     *
     * Ends tool usage and performs cleanup. If this tool has a parent, notifies
     * the parent that the child tool has ended.
     *
     * @return true if tool shutdown was successful, false otherwise.
     */
    bool EndUse();

    /**
     * @brief Checks if this tool is currently in use.
     *
     * @return true if the tool is active, false otherwise.
     */
    bool GetUse() const;

    /**
     * @brief Enables or disables this tool.
     *
     * @param enabled true to enable, false to disable the tool.
     */
    void SetEnabled(bool enabled);

    /**
     * @brief Returns a status string describing the current tool state.
     *
     * @return A wxString with current tool information for display.
     */
    virtual wxString GetString() const;

    /**
     * @brief Sets a base direction reference line for angle calculations.
     *
     * Configures a base azimuth angle that can be used as a reference for
     * angle constraints during geometry input.
     *
     * @param azimuth The base direction angle in radians.
     * @param enable true to enable base line mode, false to disable.
     */
    virtual void setBaseLine(double azimuth, bool enable);

    /**
     * @brief Checks if this tool is an input tool.
     *
     * Input tools are tools that accept coordinate/geometry input from users
     * (e.g., draw tools), as opposed to selection or manipulation tools.
     *
     * @return true if this is an input tool, false otherwise.
     */
    virtual bool isInputTool() const;

    /**
     * @brief Processes geometry selection results.
     *
     * Called when the user has selected geometry(ies) via a selection model.
     *
     * @param msg The selection result message containing selected geometries.
     */
    virtual void processSelectResult(const umodelSelectXDResultMsg &msg);

    /**
     * @brief Processes selection data query results.
     *
     * Called when the tool has queried for data properties of selected items.
     *
     * @param msg The data selection reply message with property information.
     */
    virtual void processSelectDataResult(const udataSelectReplyMsg &msg);

    /**
     * @brief Processes multi-row action notification messages.
     *
     * Called when bulk actions have been performed on multiple data items.
     *
     * @param msg The multi-row action notification message.
     */
    virtual void processActionNotify(const udataMultiRowActionNotifyMsg &msg);

    /**
     * @brief Processes keyboard commands from user input.
     *
     * Routes keyboard events to appropriate command handlers or tool-specific
     * processing.
     *
     * @param evt The keyboard event to process.
     * @return true if the event was handled, false to pass to parent handler.
     */
    bool processCommandKey(const wxKeyEvent &evt);

    /**
     * @brief Generates a human-readable description of a geometry.
     *
     * Creates a formatted string summarizing the geometry's properties
     * (type, dimensions, coordinates, etc.).
     *
     * @param g The geometry to describe.
     * @return A descriptive string for display or logging.
     */
    wxString describeGeometry(const geom::Geometry &g) const;

protected:
    /**
     * @brief Signals that a child tool has completed.
     *
     * Called by child tools to notify the parent that they are finished.
     */
    void finishChild();

    /**
     * @brief Creates and activates a child tool of the specified type.
     *
     * Template method for creating a derived tool class as a child of this tool.
     * The child tool is automatically activated.
     *
     * @tparam Tool The child tool class to create.
     */
    template <class Tool>
    void startChild()
    {
        m_childTool = std::make_unique<Tool>(m_owner);
        if (m_childTool)
        {
            m_childTool->BeginUse(this);
        }
    }

    /**
     * @brief Enables or disables the parent tool.
     *
     * @param enable true to enable the parent tool, false to disable.
     */
    void enableParent(bool enable);

    /**
     * @brief Retrieves the parent tool if one exists.
     *
     * @return Pointer to the parent tool, or nullptr if no parent exists.
     */
    gceToolBase *getParent() const;

    /**
     * @brief Retrieves the canvas rendering context.
     *
     * @return Pointer to the canvas for rendering operations.
     */
    gceCanvas *getCanvas() const;

    /**
     * @brief Called when the tool's enabled state changes.
     *
     * Derived classes can override to perform state-specific initialization.
     * Default implementation does nothing.
     */
    virtual void OnSetEnabled();

    /** @brief UI command container for tool-specific menu commands. */
    UICommandContainer uic;

    /**
     * @brief Initializes the tool before first use.
     *
     * Called by BeginUse() to perform tool-specific initialization.
     * Must be implemented by derived classes.
     */
    virtual void BeginUse_Custom() = 0;

    /**
     * @brief Finalizes and cleans up the tool after use.
     *
     * Called by EndUse() to perform tool-specific cleanup and shutdown.
     * Must be implemented by derived classes.
     *
     * @return true if shutdown was successful, false if there were issues.
     */
    virtual bool EndUse_Custom() = 0;

    /**
     * @brief Creates a tool options/properties panel.
     *
     * Generates the options panel displayed at the bottom of the editor UI
     * for configuring tool-specific parameters.
     *
     * @param parent The parent window to attach the panel to.
     * @return Pointer to the created options panel, or nullptr if no panel is needed.
     */
    virtual gceToolOptions *create_to(wxWindow *parent);

    /**
     * @brief Checks if this tool is the currently active tool.
     *
     * @return true if this tool has focus, false otherwise.
     */
    bool IsActive();

    /**
     * @brief Recalculates tool state based on user interaction.
     *
     * Called during mouse movement or other events to update the tool's
     * internal state. Override in derived classes to implement custom
     * interaction logic.
     *
     * @param mousePosition The current mouse cursor position in the canvas window.
     * @return true if the canvas needs to be redrawn, false otherwise.
     */
    virtual bool DoRecalc(const wxPoint &mousePosition);

    /**
     * @brief Renders tool-specific visual elements.
     *
     * Called to display temporary geometry, guides, dimension annotations,
     * and other visual feedback during tool usage.
     * Override in derived classes for custom rendering.
     */
    virtual void Display();

    /**
     * @brief Sets the mouse cursor for this tool.
     *
     * Updates the canvas cursor to the specified custom cursor if it differs
     * from the currently active cursor.
     *
     * @param cursor A pair of (cursor ID, wxCursor object) to set.
     */
    void SetCursor(const std::pair<int, wxCursor> &cursor);

    /**
     * @brief Sets a custom mouse cursor from XPM image data.
     *
     * Creates and sets a cursor from XPM array data with specified hotspot.
     *
     * @param cursorID A unique identifier for this cursor for caching.
     * @param xpm Array of XPM image data.
     * @param x The X coordinate of the cursor hotspot.
     * @param y The Y coordinate of the cursor hotspot.
     */
    void SetCursor(int cursorID, const char **xpm, int x, int y)
    {
        SetCursor(makeCursor(cursorID, wxImage(xpm), x, y));
    }

    /**
     * @brief Creates a cursor and associates it with an ID for caching.
     *
     * @param cursorID Unique identifier for cursor caching and reuse.
     * @param img The cursor image to use.
     * @param x X coordinate of the cursor hotspot (click point).
     * @param y Y coordinate of the cursor hotspot (click point).
     * @return A pair of (cursor ID, wxCursor object).
     */
    std::pair<int, wxCursor> makeCursor(int cursorID, wxImage img, int x, int y);

    /**
     * @brief Creates a cursor from XPM image data.
     *
     * @param cursorID Unique identifier for cursor caching and reuse.
     * @param xpm Array of XPM image data.
     * @param x X coordinate of the cursor hotspot (click point).
     * @param y Y coordinate of the cursor hotspot (click point).
     * @return A pair of (cursor ID, wxCursor object).
     */
    std::pair<int, wxCursor> makeCursor(int cursorID, const char **xpm, int x, int y)
    {
        return makeCursor(cursorID, wxImage(xpm), x, y);
    }

    /**
     * @brief Transforms geometry from world coordinates to screen coordinates.
     *
     * Projects a geometry from world/canvas coordinates to screen coordinates
     * and modifies it in-place. Used for coordinate system transformations.
     *
     * @param g The geometry to transform in-place.
     */
    void ProjectInplace(geom::Geometry &g) const;

    /**
     * @brief Transforms geometry from screen coordinates to world coordinates.
     *
     * Inverse projects a geometry from screen coordinates to world/canvas coordinates
     * and modifies it in-place.
     *
     * @param g The geometry to transform in-place.
     */
    void UnProjectInplace(geom::Geometry &g) const;

    /**
     * @brief Handles mouse motion events.
     *
     * @param event The wxMouseEvent containing motion details.
     */
    void OnMotion(wxMouseEvent &event);

    /**
     * @brief Updates the tool status string display.
     */
    void updateToolString();

    /**
     * @brief Handles right mouse button release events.
     *
     * @param event The wxMouseEvent containing release details.
     */
    void OnRightUp(wxMouseEvent &event);

    /**
     * @brief Handles paint events for rendering tool graphics.
     *
     * @param event The wxPaintEvent.
     */
    void OnPaint(wxPaintEvent &event);

    /**
     * @brief Performs the actual painting of tool graphics.
     */
    void DoPaint();

    /** @brief Reference to the editor frame that owns this tool. */
    gceEditorFrame &m_owner;

    /**
     * @brief Checks if the mouse cursor is currently over the canvas.
     *
     * @return true if mouse is in canvas, false otherwise.
     */
    bool HaveMouse() const;

    /** @brief Flag indicating if this tool is currently in use. */
    bool m_fInUse = false;

    /**
     * @brief Ends the child tool if one exists.
     *
     * @return true if successful or no child exists, false if child end failed.
     */
    bool EndUseChild()
    {
        return (m_childTool ? m_childTool->EndUse() : true);
    }

    /**
     * @brief Updates the display of the tool options panel.
     */
    void updateOtionsPanel();

    /**
     * @brief Generates a string representing records/counts.
     *
     * @return A formatted string with record information.
     */
    wxString getRecordsString() const;

    /**
     * @brief Posts selection model properties for rendering.
     *
     * Sends selection data to the renderer for display of highlighted
     * or selected geometry with optional point marking.
     *
     * @param drawPoints true to render vertex points, false otherwise.
     * @param pose The transformation matrix for the selection.
     * @param poseArray Optional array of additional transformation matrices.
     */
    void postSelectionModelProps(bool drawPoints, const glm::dmat4 &pose, const std::vector<glm::dmat4> &poseArray = {});

private:
    /** @brief Parent tool if this is a child tool, nullptr otherwise. */
    gceToolBase *m_parentTool = nullptr;

    /** @brief Child tool if one is active, nullptr otherwise. */
    std::unique_ptr<gceToolBase> m_childTool;

    /** @brief Tool options panel displayed in the UI, nullptr if not used. */
    gceToolOptions *m_optpanel = nullptr;

    /** @brief Currently active cursor (ID and wxCursor object pair). */
    std::pair<int, wxCursor> m_currentCursor = {-1, {}};
};
