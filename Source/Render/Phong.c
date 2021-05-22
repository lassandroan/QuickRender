// Copyright (C) 2021  Antonio Lassandro

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.

static void
RenderPhong(
          RenderContext * const context,
    const Matrix        * const model_view_projection)
{
    SDL_assert(context);
    SDL_assert(model_view_projection);

    Mesh * const mesh = context->mesh;
    for (size_t i = 0; i < mesh->faces.size; ++i)
    {
        const Face * const face = &mesh->faces.data[i];

        Vector verts[3];
        Vector norms[3];
        for (size_t j = 0; j < 3; ++j)
        {
            verts[j] = mesh->vertices.data[face->indices[j].v];
            verts[j].y *= -1.0f;
        }

        if (!TestBackface(context, verts))
            continue;

        for (size_t j = 0; j < 3; ++j)
        {
            norms[j] = mesh->normals.data[face->indices[j].n];

            Matrix vert_mat = VectorToMatrix(&verts[j]);
                   vert_mat = MatrixMult(model_view_projection, &vert_mat);
                   verts[j] = MatrixToVector(&vert_mat);

            verts[j].x = roundf(verts[j].x);
            verts[j].y = roundf(verts[j].y);
            verts[j].z = roundf(verts[j].z);
        }

        if (verts[0].y > verts[1].y)
        {
            const Vector vtmp = verts[0]; verts[0] = verts[1]; verts[1] = vtmp;
            const Vector ntmp = norms[0]; norms[0] = norms[1]; norms[1] = ntmp;
        }
        if (verts[0].y > verts[2].y)
        {
            const Vector vtmp = verts[0]; verts[0] = verts[2]; verts[2] = vtmp;
            const Vector ntmp = norms[0]; norms[0] = norms[2]; norms[2] = ntmp;
        }
        if (verts[1].y > verts[2].y)
        {
            const Vector vtmp = verts[1]; verts[1] = verts[2]; verts[2] = vtmp;
            const Vector ntmp = norms[1]; norms[1] = norms[2]; norms[2] = ntmp;
        }

        const SDL_FRect bounds = TriBoundingBox(verts);

        Vector point;
        for (point.x = bounds.x; point.x <= bounds.x + bounds.w; point.x += 0.5f)
        {
            for (point.y = bounds.y; point.y <= bounds.y + bounds.h; point.y += 0.5f)
            {
                const Vector coord = Barycenter(verts, &point);
                if (coord.x < 0.0f || coord.y < 0.0f || coord.z < 0.0f)
                    continue;

                point.z = 0.0f;
                for (size_t i = 0; i < 3; ++i)
                    point.z += verts[i].z * coord.xyz[i];

                if (!TestDepth(context, &point))
                    continue;

                Vector interp_norm = {{.x = 0.0f}};
                for (size_t i = 0; i < 3; ++i)
                {
                    Vector norm = VectorMultf(&norms[i], coord.xyz[i]);
                    interp_norm = VectorAdd(&interp_norm, &norm);
                }

                interp_norm = VectorNormalize(&interp_norm);

                float interp_color = VectorDot(
                    &interp_norm,
                    &context->light
                );

                interp_color = fmaxf(fminf(interp_color, 1.0f), 0.0f) * 255.0f;

                PutFragment(
                    context,
                    &point,
                    &(SDL_Color){
                        (uint8_t)interp_color,
                        (uint8_t)interp_color,
                        (uint8_t)interp_color,
                        255,
                    }
                );
            }
        }
    }
}
