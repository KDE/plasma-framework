#ifndef SVGTEXTURENODE_H
#define SVGTEXTURENODE_H

#include <QSGSimpleTextureNode>

namespace Plasma {

/**
* This class wraps QSGSimpleTextureNode
* and manages the lifespan on the texture
*/

class SVGTextureNode : public QSGSimpleTextureNode
{
    public:
        SVGTextureNode() {}
        /**
         * Set the current texture
         * the object takes ownership of the texture
         */
        void setTexture(QSGTexture *texture) {
            m_texture.reset(texture);
            QSGSimpleTextureNode::setTexture(texture);
        }
    private:
        QScopedPointer<QSGTexture> m_texture;
};

}
#endif // SVGTextureNode
