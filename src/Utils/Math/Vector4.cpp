//
// Created by Monika on 1.10.2024.
//

#include <Utils/Math/Vector4.h>

#include <Utils/Common/StringAtomLiterals.h>

namespace SR_MATH_NS {
    std::unordered_map<SRHashType, FColor> SR_COLOR_PALETTE = {
        { "transparent"_atom_hash, FColor(0, 0, 0, 0) / 255.f },
        { "black"_atom_hash, FColor(0, 0, 0, 255) / 255.f },
        { "silver"_atom_hash, FColor(192, 192, 192, 255) / 255.f },
        { "gray"_atom_hash, FColor(128, 128, 128, 255) / 255.f },
        { "white"_atom_hash, FColor(255, 255, 255, 255) / 255.f },
        { "maroon"_atom_hash, FColor(128, 0, 0, 255) / 255.f },
        { "red"_atom_hash, FColor(255, 0, 0, 255) / 255.f },
        { "purple"_atom_hash, FColor(128, 0, 128, 255) / 255.f },
        { "fuchsia"_atom_hash, FColor(255, 0, 255, 255) / 255.f },
        { "green"_atom_hash, FColor(0, 128, 0, 255) / 255.f },
        { "lime"_atom_hash, FColor(0, 255, 0, 255) / 255.f },
        { "olive"_atom_hash, FColor(128, 128, 0, 255) / 255.f },
        { "yellow"_atom_hash, FColor(255, 255, 0, 255) / 255.f },
        { "navy"_atom_hash, FColor(0, 0, 128, 255) / 255.f },
        { "blue"_atom_hash, FColor(0, 0, 255, 255) / 255.f },
        { "teal"_atom_hash, FColor(0, 128, 128, 255) / 255.f },
        { "aqua"_atom_hash, FColor(0, 255, 255, 255) / 255.f },
        { "pink"_atom_hash, FColor(255, 192, 203, 255) / 255.f },
        { "brown"_atom_hash, FColor(165, 42, 42, 255) / 255.f },
        { "orange"_atom_hash, FColor(255, 165, 0, 255) / 255.f },
        { "coral"_atom_hash, FColor(255, 127, 80, 255) / 255.f },
        { "gold"_atom_hash, FColor(255, 215, 0, 255) / 255.f },
        { "beige"_atom_hash, FColor(245, 245, 220, 255) / 255.f },
        { "ivory"_atom_hash, FColor(255, 255, 240, 255) / 255.f },
        { "khaki"_atom_hash, FColor(240, 230, 140, 255) / 255.f },
        { "tan"_atom_hash, FColor(210, 180, 140, 255) / 255.f },
        { "wheat"_atom_hash, FColor(245, 222, 179, 255) / 255.f },
        { "azure"_atom_hash, FColor(240, 255, 255, 255) / 255.f },
        { "cyan"_atom_hash, FColor(0, 255, 255, 255) / 255.f },
        { "magenta"_atom_hash, FColor(255, 0, 255, 255) / 255.f },
        { "violet"_atom_hash, FColor(238, 130, 238, 255) / 255.f },
        { "indigo"_atom_hash, FColor(75, 0, 130, 255) / 255.f },
        { "turquoise"_atom_hash, FColor(64, 224, 208, 255) / 255.f },
        { "skyblue"_atom_hash, FColor(135, 206, 235, 255) / 255.f },
        { "royalblue"_atom_hash, FColor(65, 105, 225, 255) / 255.f },
        { "slateblue"_atom_hash, FColor(106, 90, 205, 255) / 255.f },
        { "steelblue"_atom_hash, FColor(70, 130, 180, 255) / 255.f },
        { "powderblue"_atom_hash, FColor(176, 224, 230, 255) / 255.f },
        { "aliceblue"_atom_hash, FColor(240, 248, 255, 255) / 255.f },
        { "ghostwhite"_atom_hash, FColor(248, 248, 255, 255) / 255.f },
        { "lavender"_atom_hash, FColor(230, 230, 250, 255) / 255.f },
        { "mintcream"_atom_hash, FColor(245, 255, 250, 255) / 255.f },
        { "honeydew"_atom_hash, FColor(240, 255, 240, 255) / 255.f },
        { "seashell"_atom_hash, FColor(255, 245, 238, 255) / 255.f },
        { "floralwhite"_atom_hash, FColor(255, 250, 240, 255) / 255.f },
        { "linen"_atom_hash, FColor(250, 240, 230, 255) / 255.f },
        { "oldlace"_atom_hash, FColor(253, 245, 230, 255) / 255.f },
        { "papayawhip"_atom_hash, FColor(255, 239, 213, 255) / 255.f },
        { "blanchedalmond"_atom_hash, FColor(255, 235, 205, 255) / 255.f },
        { "bisque"_atom_hash, FColor(255, 228, 196, 255) / 255.f },
        { "peachpuff"_atom_hash, FColor(255, 218, 185, 255) / 255.f },
        { "mistyrose"_atom_hash, FColor(255, 228, 225, 255) / 255.f },
        { "antiquewhite"_atom_hash, FColor(250, 235, 215, 255) / 255.f },
        { "beige"_atom_hash, FColor(245, 245, 220, 255) / 255.f },
        { "cornsilk"_atom_hash, FColor(255, 248, 220, 255) / 255.f },
        { "lemonchiffon"_atom_hash, FColor(255, 250, 205, 255) / 255.f },
        { "lightgoldenrodyellow"_atom_hash, FColor(250, 250, 210, 255) / 255.f },
        { "lightyellow"_atom_hash, FColor(255, 255, 224, 255) / 255.f },
        { "palegoldenrod"_atom_hash, FColor(238, 232, 170, 255) / 255.f },
        { "khaki"_atom_hash, FColor(240, 230, 140, 255) / 255.f },
    };
}