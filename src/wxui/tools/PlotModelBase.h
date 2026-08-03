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
#include <vector>
#include "geom/Coordinate.h"
#include "geom/LineSegment.h"

namespace geom
{
class CoordinateSeq;
class Geometry;
};

class gceCanvas;

/**
 * @brief Sticky constraint mode: no snapping to any geometry.
 */
#define GCE_PM_STICK_TO_NONE 0.0

/**
 * @brief Sticky constraint mode: snap to vertices/nodes.
 */
#define GCE_PM_STICK_TO_VERTEX 1.0

/**
 * @brief Sticky constraint mode: snap to line segments.
 */
#define GCE_PM_STICK_TO_SEGMENT 2.0

/**
 * @brief Sticky constraint mode: snap to line closure (connecting to start point).
 */
#define GCE_PM_STICK_TO_CLOSURE 3.0

/**
 * @class PlotConstraint
 * @brief Abstract base class for coordinate input constraints during geometry editing.
 *
 * PlotConstraint defines the interface for applying geometric constraints and snapping
 * behaviors when drawing or editing geometries interactively. Derived classes implement
 * specific constraint types (empty, point, ray, circle) to guide coordinate input.
 */
class PlotConstraint
{
public:
    /**
     * @brief Finds intersection with a line segment from a current point.
     *
     * Computes the intersection between this constraint and a line segment defined
     * by its endpoints.
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The current point from which the line segment starts.
     * @param ls The line segment to test for intersection.
     * @return true if an intersection was found, false otherwise.
     */
    virtual bool Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const = 0;

    /**
     * @brief Finds intersection with a ray from a single point.
     *
     * Computes the intersection between this constraint and a ray emanating from
     * the given coordinate.
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The starting point of the ray.
     * @return true if an intersection was found, false otherwise.
     */
    virtual bool Intersection(geom::Coordinate &dest, const geom::Coordinate &cpt) const = 0;

    /**
     * @brief Projects a coordinate onto this constraint.
     *
     * Calculates the closest point on this constraint to the given coordinate.
     *
     * @param nextCoord The coordinate to project.
     * @return The projected coordinate on this constraint.
     */
    virtual geom::Coordinate Project(geom::Coordinate &nextCoord) const = 0;

    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~PlotConstraint() {}
};

/**
 * @class PlotConstraint_Empty
 * @brief A constraint that applies no restrictions (unconstrained input).
 *
 * This constraint type represents a state where no special geometric constraints
 * are applied to coordinate input, allowing free placement of points.
 */
class PlotConstraint_Empty : public PlotConstraint
{
public:

    /**
     * @brief Finds intersection with a line segment (no constraint).
     *
     * @param dest Output parameter for the coordinate.
     * @param cpt The current point.
     * @param ls The line segment.
     * @return Always returns false as no constraint is applied.
     */
    virtual bool Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const;

    /**
     * @brief Finds intersection with a ray (no constraint).
     *
     * @param dest Output parameter for the coordinate.
     * @param cpt The starting point.
     * @return Always returns false as no constraint is applied.
     */
    virtual bool Intersection(geom::Coordinate &dest, const geom::Coordinate &cpt) const;

    /**
     * @brief Projects a coordinate (returns unchanged).
     *
     * @param nextCoord The coordinate to project.
     * @return The same coordinate unchanged.
     */
    virtual geom::Coordinate Project(geom::Coordinate &nextCoord) const;
};
/**
 * @class PlotConstraint_Point
 * @brief A constraint that restricts input to a single fixed point.
 *
 * This constraint snaps coordinates to a specific point in space,
 * used for constraining geometry to pass through a fixed location.
 */
class PlotConstraint_Point : public PlotConstraint
{
public:
    /**
     * @brief Constructs a point constraint.
     *
     * @param pos The position in 3D space to which coordinates will be constrained.
     */
    explicit PlotConstraint_Point(const glm::dvec3 &pos) : m_pos(pos) {}

    /**
     * @brief Finds intersection with a line segment (returns the constraint point).
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The current point.
     * @param ls The line segment.
     * @return true if the constraint point lies on the line segment.
     */
    virtual bool Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const;

    /**
     * @brief Finds intersection with a ray (returns the constraint point).
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The starting point.
     * @return true if the constraint point lies on the ray.
     */
    virtual bool Intersection(geom::Coordinate &dest, const geom::Coordinate &cpt) const;

    /**
     * @brief Projects a coordinate (returns the constraint point).
     *
     * @param nextCoord The coordinate to project.
     * @return The constraint point.
     */
    virtual geom::Coordinate Project(geom::Coordinate &) const;

private:
    /** @brief The fixed point that defines this constraint. */
    glm::dvec3 m_pos;
};

/**
 * @class PlotConstraint_Ray
 * @brief A constraint that restricts input to a ray (half-infinite line).
 *
 * This constraint snaps coordinates to a ray emanating from a starting point
 * in a specified direction (azimuth).
 */
class PlotConstraint_Ray : public PlotConstraint
{
public:
    /**
     * @brief Constructs a ray constraint.
     *
     * @param pos The starting point of the ray in 3D space.
     * @param azi The azimuth (bearing) angle in radians that defines the ray direction.
     */
    PlotConstraint_Ray(const glm::dvec3 &pos, double azi) : m_pos(pos), m_azi(azi) {}

    /**
     * @brief Finds intersection with a line segment.
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The current point.
     * @param ls The line segment.
     * @return true if an intersection between the ray and line segment exists.
     */
    virtual bool Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const;

    /**
     * @brief Finds intersection with a ray (returns projection onto this ray).
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The starting point.
     * @return true if a valid projection exists.
     */
    virtual bool Intersection(geom::Coordinate &dest, const geom::Coordinate &cpt) const;

    /**
     * @brief Projects a coordinate onto this ray.
     *
     * @param nextCoord The coordinate to project.
     * @return The projected coordinate on this ray.
     */
    virtual geom::Coordinate Project(geom::Coordinate &nextCoord) const;

private:
    /** @brief The starting point of the ray. */
    glm::dvec3 m_pos;

    /** @brief The azimuth (bearing) angle defining the ray direction. */
    double m_azi;
};

/**
 * @class PlotConstraint_Circle
 * @brief A constraint that restricts input to a circle in 3D space.
 *
 * This constraint snaps coordinates to a circular path. The circle is defined
 * by its center point and radius.
 */
class PlotConstraint_Circle final : public PlotConstraint
{
public:
    /**
     * @brief Constructs a circle constraint.
     *
     * @param cen The center point of the circle in 3D space.
     * @param R The radius of the circle.
     */
    PlotConstraint_Circle(const glm::dvec3 &cen, double R) : m_circle(cen, R) {}

    /**
     * @brief Finds intersection with a line segment.
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The current point.
     * @param ls The line segment.
     * @return true if an intersection between the circle and line segment exists.
     */
    virtual bool Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const;

    /**
     * @brief Finds intersection with a ray.
     *
     * @param dest Output parameter for the intersection coordinate.
     * @param cpt The starting point.
     * @return true if an intersection between the circle and ray exists.
     */
    virtual bool Intersection(geom::Coordinate &dest, const geom::Coordinate &cpt) const;

    /**
     * @brief Projects a coordinate onto this circle.
     *
     * @param nextCoord The coordinate to project.
     * @return The projected coordinate on the circle circumference.
     */
    virtual geom::Coordinate Project(geom::Coordinate &nextCoord) const;

private:
    /** @brief Coordinate representing the circle (center and radius information). */
    geom::Coordinate m_circle;
};

/**
 * @enum gceEditorDecoration
 * @brief Enumeration of visual decoration identifiers for the geometry editor.
 *
 * These constants identify different visual elements rendered during geometry editing.
 */
enum gceEditorDecoration
{
    MAIN_LINE_ID = 1,      /*!< Identifier for the main edited geometry line */
    TEMP_LINE_ID1 = 2,     /*!< Identifier for the first temporary guide line */
    TEMP_LINE_ID2 = 3,     /*!< Identifier for the second temporary guide line */
};

/**
 * @class gcePlotModelBase
 * @brief Base class for interactive geometry editing and plotting models.
 *
 * gcePlotModelBase provides the framework for interactive coordinate input, constraint application,
 * snapping behavior, and visual feedback during geometry editing. It handles angle/length constraints,
 * geometry snapping, and temporary visualization of work-in-progress geometry.
 *
 * The class manages:
 * - Coordinate input constraints (angle step, base angle, length step)
 * - Geometry snapping to existing vertices and segments
 * - Temporary rendering of construction geometry
 * - User interaction feedback and coordinate recalculation
 *
 * @note This class is non-copyable (inherits from boost::noncopyable).
 */
class gcePlotModelBase : boost::noncopyable
{
public:
    /**
     * @brief Constructs a plot model with a rendering context.
     *
     * @param ctx Pointer to the canvas context for rendering operations.
     */
    explicit gcePlotModelBase(gceCanvas *ctx);

    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~gcePlotModelBase();

    /**
     * @brief Creates an appropriate constraint based on coordinate sequence context.
     *
     * Generates a constraint object that will guide subsequent coordinate input
     * based on the current sequence state and next coordinate.
     *
     * @param seq The current coordinate sequence.
     * @param next The proposed next coordinate.
     * @return A unique_ptr to a PlotConstraint instance.
     */
    virtual std::unique_ptr<PlotConstraint> CreateConstraint(const geom::CoordinateSeq &seq, const geom::Coordinate &next) const;

    // Configuration flags and parameters for input constraints

    /**
     * @brief Enable/disable angular step constraint.
     *
     * When true, the angle between consecutive segments is constrained to a fixed step.
     */
    bool m_fAngleStep = false;

    /**
     * @brief Angular step value in degrees.
     *
     * The fixed angle increment when angular step constraint is enabled (default 45°).
     */
    double m_angle = 45.0;

    /**
     * @brief Enable/disable base angle constraint.
     *
     * When true, angles are calculated relative to a base direction rather than
     * the previous segment.
     */
    bool m_fBase = false;

    /**
     * @brief Base angle value in radians.
     *
     * The reference direction angle for base angle constraint calculation.
     */
    double m_base = 0.0;

    /**
     * @brief Enable/disable length step constraint.
     *
     * When true, segment lengths are constrained to a fixed step value.
     */
    bool m_fLength = false;

    /**
     * @brief Length step value in coordinate units (typically meters).
     *
     * The fixed length increment when length step constraint is enabled.
     */
    double m_lstep = 0.01;

    /**
     * @brief Recalculates local variables for display and coordinate input.
     *
     * Updates the current coordinate position based on mouse position and applies
     * constraints. Generates hint sequence for snapping and visual feedback.
     *
     * @param seq The current coordinate sequence being edited.
     * @param mousePosition The current mouse cursor position in pixels.
     * @param hint_seq Output: sequence of snapping hint coordinates.
     * @return The recalculated input coordinate with snapping applied.
     *
     * @note This is a pure virtual function that must be implemented by derived classes.
     * @todo Consider using a dedicated struct for hint_seq with stick mode and other metadata.
     */
    virtual geom::Coordinate DoRecalc(const geom::CoordinateSeq &seq, const wxPoint &mousePosition, std::vector<geom::Coordinate> &hint_seq) = 0;

    /**
     * @brief Adds a new point to the coordinate sequence with partial copy support (not implemented).
     *
     * Processes the new point addition, potentially copying segments from neighboring
     * geometries when partial copy mode is enabled.
     *
     * @param seq The coordinate sequence to modify.
     * @param next The new coordinate point to add.
     * @return The actual number of points added (may be > 1 with partial copy).
     */
    virtual size_t AddPoint(geom::CoordinateSeq &seq, const geom::Coordinate &next);

    /**
     * @brief Posts a temporary guide line from the last entered point to the cursor.
     *
     * Renders a color-inverted line segment from the end of the current sequence
     * to the current cursor position for visual feedback during editing.
     *
     * @param seq The current coordinate sequence.
     * @param next The next coordinate being previewed.
     * @param haveMouse true if the mouse cursor is currently within the canvas.
     */
    virtual void postRenderTempLine1(const geom::CoordinateSeq &seq, const geom::Coordinate &next, bool haveMouse);

    /**
     * @brief Posts a dotted temporary guide line from the first point to the cursor.
     *
     * Renders a dotted, color-inverted line segment from the start of the current sequence
     * to the current cursor position. Used for ring closure visualization.
     *
     * @param seq The current coordinate sequence.
     * @param next The next coordinate being previewed.
     * @param haveMouse true if the mouse cursor is currently within the canvas.
     */
    virtual void postRenderTempLine2(const geom::CoordinateSeq &seq, const geom::Coordinate &next, bool haveMouse);

    /**
     * @brief Posts a temporary line to the renderer with optional stipple pattern.
     *
     * Low-level method for rendering a temporary construction line between two coordinates.
     *
     * @param TEMP_LINE_ID The line identifier for rendering.
     * @param coo Reference starting coordinate for the line.
     * @param cursorCoord The ending coordinate at the cursor position.
     * @param useStipple true to render with dashed line pattern, false for solid.
     */
    void postRenderTempLine(const uint32_t TEMP_LINE_ID, geom::Coordinate &coo, const geom::Coordinate &cursorCoord, bool useStipple);

    /**
     * @brief Posts the main edited geometry line to the renderer.
     *
     * Renders the complete coordinate sequence representing the geometry being edited.
     *
     * @param lineId The line identifier for rendering.
     * @param seq The coordinate sequence to render.
     * @param width The line width in pixels.
     * @param useStipple true for dashed line pattern, false for solid.
     */
    void postRenderLine(uint32_t lineId, const geom::CoordinateSeq &seq, float width, bool useStipple);

    /**
     * @brief Clears a previously rendered line from the display.
     *
     * Removes a temporary or guide line from the renderer.
     *
     * @param lineId The line identifier to clear.
     */
    void postRenderClearLine(const uint32_t lineId);

    /**
     * @brief Posts the main entered geometry ring to the editor model.
     *
     * Default implementation renders the coordinate sequence as the primary edited geometry.
     * Can be overridden by derived classes for custom visualization.
     *
     * @param seq The coordinate sequence representing the main geometry.
     */
    virtual void postModelMainLine(const geom::CoordinateSeq &seq)
    {
        postRenderLine(MAIN_LINE_ID, seq, 2.0, false);
    }

    /**
     * @brief Returns information about the current segment for user display.
     *
     * Generates a human-readable string describing the current segment being drawn,
     * including length and angle information based on constraint configuration.
     *
     * When m_fBase is true, shows angle relative to base direction;
     * when false, shows angle relative to previous segment.
     *
     * @param seq The current coordinate sequence.
     * @param nextCoo The next coordinate being input.
     * @return A formatted string with segment information and units.
     */
    wxString GetString(const geom::CoordinateSeq &seq, const geom::Coordinate &nextCoo) const;

    /**
     * @brief Prompts the user to interactively set the length of the next segment.
     *
     * Opens a dialog allowing the user to specify or confirm the length value.
     *
     * @param seq The current coordinate sequence.
     * @param parent The parent window for the dialog.
     * @param nextCoord Output: updated coordinate with the specified length applied.
     * @return true if the user confirmed the length, false if cancelled.
     */
    bool setLength_User(const geom::CoordinateSeq &seq, wxWindow *parent, geom::Coordinate &nextCoord);

    /**
     * @brief Calculates formatted distance between two coordinates.
     *
     * Computes the distance in the context's spatial reference system (projection)
     * and formats it as a human-readable string with appropriate units.
     *
     * @param A The first coordinate in the context's projection coordinate system.
     * @param B The second coordinate in the context's projection coordinate system.
     * @return A formatted distance string (e.g., "123.45 m", "1.23 km").
     */
    wxString distanceHR(const geom::Coordinate &A, const geom::Coordinate &B) const;

protected:
    /**
     * @brief Sets the snapping mode indicator for a coordinate sequence.
     *
     * Encodes the snapping result into the coordinate sequence (via Coordinate metadata)
     * to indicate what type of snapping was performed:
     * - GCE_PM_STICK_TO_NONE (0.0): No snapping
     * - GCE_PM_STICK_TO_VERTEX (1.0): Snapped to a vertex/node
     * - GCE_PM_STICK_TO_SEGMENT (2.0): Snapped to a line segment
     * - GCE_PM_STICK_TO_CLOSURE (3.0): Snapped to the start point (line closure)
     *
     * @param seq The coordinate sequence to update.
     * @param mode The snapping mode value to set.
     */
    static void set_stick_mode(std::vector<geom::Coordinate> &seq, double mode);

public://protected:
    /** @brief Pointer to the canvas rendering context. */
    gceCanvas *m_canvas = nullptr;
};

