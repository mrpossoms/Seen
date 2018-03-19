#pragma once
#include "seen.hpp"

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
