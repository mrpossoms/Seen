#pragma once
#include "seen.hpp"

#define SKY_SHADERS { .vertex = "sky.vsh", .fragment = "sky.fsh" }

namespace seen
{
    class Sky : public Drawable {
    public:
        Sky();
        ~Sky();

        void draw(Viewer* viewer);
    private:
        GLuint vbo;
        int vertices;
    };
}
