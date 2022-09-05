# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

qt6_add_shaders(corebindingsplugin "shaders6"
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    NOMSL
    NOHLSL
    PREFIX "/plasma-framework/shaders/"
    FILES
        fadingmaterial.vert
        fadingmaterial.frag
)
