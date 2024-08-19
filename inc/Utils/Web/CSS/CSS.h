//
// Created by Monika on 15.08.2024.
//

#ifndef SR_COMMON_WEB_CSS_H
#define SR_COMMON_WEB_CSS_H

#include <Utils/Math/Vector2.h>
#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS::Web {
    struct CSSSizeValue {
        float_t value = 0.0f;
        enum class Unit : uint8_t {
            Px,
            Percent,
        } unit = Unit::Px;
    };

    enum class CSSPosition : uint8_t {
        Absolute,
        Relative,
        Fixed,
        Static,
        Sticky,
    };

    enum class CSSDisplay : uint8_t {
        /** <display-outside> values */
        Block, Inline, RunIn,

        /** <display-inside> values */
        Flow, FlowRoot, Table, Flex, Grid, Ruby,

        /** <display-outside> plus <display-inside> values */
        BlockFlow, InlineTable, FlexRunIn,

        /** <display-listitem> values */
        ListItem, ListItemBlock, ListItemInline, ListItemFlow,
        ListItemFlowRoot, ListItemBlockFlow, ListItemBlockFlowRoot,
        FlowListItemBlock,

        /** <display-internal> values */
        TableRowGroup, TableHeaderGroup, TableFooterGroup,
        TableRow, TableCell, TableColumnGroup, TableColumn,
        TableCaption, RubyBase, RubyText, RubyBaseContainer,
        RubyTextContainer,

        /** <display-box> values */
        Contents, None,

        /** <display-legacy> values */
        InlineBlock, InlineFlex, InlineGrid,

        /** Global values */
        Inherit, Initial, Unset,
    };

    enum class CSSGradientSide : uint8_t {
        ClosestSide, ClosestCorner, FarthestSide, FarthestCorner,
    };

    struct CSSDirection {
        enum class DirectionType : uint8_t {
            To, Degree,
        } directionType = DirectionType::To;

        enum class Direction : uint8_t {
            Top, Right, Bottom, Left,
            TopRight, BottomRight, BottomLeft, TopLeft,
        };

        union {
            Direction direction = Direction::Bottom;
            float_t degree;
        };
    };

    struct CSSColor;

    struct CSSRGBAColor {
        uint8_t r = 0, g = 0, b = 0, a = 0;
    };

    struct CSSGradientColor {
        std::vector<CSSColor*> colors;
        CSSSizeValue size;
        bool hasSize = false;
    };

    struct CSSLinearGradient {
        CSSDirection direction;
        std::vector<CSSGradientColor> colors;
    };

    struct CSSRadialGradient {
        CSSSizeValue at;
        std::vector<CSSGradientColor> colors;

        enum class Shape : uint8_t {
            Circle, Ellipse,
        } shape = Shape::Circle;

        CSSGradientSide side = CSSGradientSide::ClosestSide;
    };

    struct CSSColor {
        enum class ColorType : uint8_t {
            RGBA, LinearGradient, RadialGradient,
        };

        struct Color {
            ColorType colorType = ColorType::RGBA;
            void* pColor = nullptr;
        };

        std::vector<Color> colors;
    };

    struct CSSBoxModelProperties {
        CSSSizeValue marginTop, borderTop, paddingTop;
        CSSSizeValue marginLeft, borderLeft, paddingLeft;
        CSSSizeValue marginRight, borderRight, paddingRight;
        CSSSizeValue marginBottom, borderBottom, paddingBottom;
        CSSSizeValue width, height;

        /**                    margin-top
         *                     border-top
         *                     padding-top
         *  m    b     p                        p      b      m
         *  -    -     -         content        -      -      -
         *  left left  left                     right  right  right
         *                    padding-bottom
         *                    border-bottom
         *                    margin-bottom
         **/
    };
}

#endif //SR_COMMON_WEB_CSS_H
