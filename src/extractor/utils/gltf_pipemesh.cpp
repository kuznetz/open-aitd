#pragma once
#include "my_gltf.h"

tinygltf::Primitive createPipePrim(tinygltf::Model& m, Vector3 points[], float radius, int sides, int material) {
    const int pointCount = 2; // As specified in your comment

    int vertexCount = pointCount * sides;
    int triangleCount = (pointCount - 1) * sides * 2;

    // Generate vertices
    std::vector<Vector3> vertices(vertexCount);

    // Generate vertices for each segment
    for (int i = 0; i < pointCount; i++) {
        Vector3 forward;
        if (i == 0) {
            // First point - use direction to next point
            forward = Vector3Normalize(Vector3Subtract(points[i + 1], points[i]));
        }
        else if (i == pointCount - 1) {
            // Last point - use direction from previous point
            forward = Vector3Normalize(Vector3Subtract(points[i], points[i - 1]));
        }
        else {
            // Middle points - average direction
            Vector3 dir1 = Vector3Normalize(Vector3Subtract(points[i], points[i - 1]));
            Vector3 dir2 = Vector3Normalize(Vector3Subtract(points[i + 1], points[i]));
            forward = Vector3Normalize(Vector3Add(dir1, dir2));
        }

        // Calculate right and up vectors
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, { 0, 1, 0 }));
        if (Vector3Length(right) < 0.001f) {
            right = Vector3Normalize(Vector3CrossProduct(forward, { 1, 0, 0 }));
        }
        Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

        // Generate circle of vertices around the point
        for (int j = 0; j < sides; j++) {
            float angle = 2 * PI * j / sides;
            Vector3 radial = Vector3Add(
                Vector3Scale(right, cosf(angle) * radius),
                Vector3Scale(up, sinf(angle) * radius)
            );
            vertices[i * sides + j] = Vector3Add(points[i], radial);
        }
    }

    // Generate indices for triangles
    std::vector<unsigned int> indices(triangleCount * 3);
    int index = 0;
    for (int i = 0; i < pointCount - 1; i++) {
        for (int j = 0; j < sides; j++) {
            int nextJ = (j + 1) % sides;
            // First triangle
            indices[index++] = i * sides + j;
            indices[index++] = (i + 1) * sides + j;
            indices[index++] = i * sides + nextJ;
            // Second triangle
            indices[index++] = i * sides + nextJ;
            indices[index++] = (i + 1) * sides + j;
            indices[index++] = (i + 1) * sides + nextJ;
        }
    }

    // Create vertex buffer accessor
    int vertAccIdx = createVertexes(m, vertices);
    // Create primitive
    return createPolyPrimitive(m, indices, vertAccIdx, material);
}