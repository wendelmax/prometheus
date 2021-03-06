#ifndef ROOM_HPP
#define ROOM_HPP

#include <cassert>
#include <filesystem>
#include <string>
#include <vector>

#include "lights.hpp"
#include "logger.hpp"
#include "shader.hpp"
#include "shapes.hpp"
#include "utility.hpp"

const std::vector<float> floor_vertices = {
    // positions         // normals          // texture coords
     0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  4.0f, 4.0f,  // top right
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  4.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 4.0f,  // top left
};
const std::vector<float> wall_vertices = {
    // positions         // normals          // texture coords
     0.5f,  0.25f, 0.0f,  0.0f, 0.0f, -1.0f,  4.0f, 2.0f,  // top right
     0.5f, -0.25f, 0.0f,  0.0f, 0.0f, -1.0f,  4.0f, 0.0f,  // bottom right
    -0.5f, -0.25f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,  // bottom left
    -0.5f,  0.25f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 2.0f,  // top left
};

class Room
{
public:
    Room(std::filesystem::path floor_diffuse_texture_path_,
        std::filesystem::path floor_specular_texture_path_,
        std::filesystem::path ceiling_diffuse_texture_path_,
        std::filesystem::path ceiling_specular_texture_path_,
        std::filesystem::path wall_diffuse_texture_path_,
        std::filesystem::path wall_specular_texture_path_,
        SceneLighting* scene_lighting_,
        float scale_factor_,
        glm::vec3 dimensions_,
        glm::vec3 position_) :
            floor_diffuse_texture_path(floor_diffuse_texture_path_),
            floor_specular_texture_path(floor_specular_texture_path_),
            ceiling_diffuse_texture_path(ceiling_diffuse_texture_path_),
            ceiling_specular_texture_path(ceiling_specular_texture_path_),
            wall_diffuse_texture_path(wall_diffuse_texture_path_),
            wall_specular_texture_path(wall_specular_texture_path_),
            sl(scene_lighting_),
            scale_factor(scale_factor_),
            dimensions(dimensions_),
            position(position_)
    {
    }

    void init();
    void deinit();
    void draw(Shader* shader);

    void set_depth_map(unsigned int);
private:
    std::filesystem::path floor_diffuse_texture_path;
    std::filesystem::path floor_specular_texture_path;
    std::filesystem::path ceiling_diffuse_texture_path;
    std::filesystem::path ceiling_specular_texture_path;
    std::filesystem::path wall_diffuse_texture_path;
    std::filesystem::path wall_specular_texture_path;

    unsigned int floor_diffuse_texture;
    unsigned int floor_specular_texture;
    unsigned int ceiling_diffuse_texture;
    unsigned int ceiling_specular_texture;
    unsigned int wall_diffuse_texture;
    unsigned int wall_specular_texture;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    SceneLighting* sl;

    float scale_factor;
    glm::vec3 dimensions;
    glm::vec3 position;

    /*
     * Transformation matrices.
     */
    // Floor.
    glm::vec3 floor_translation_vec;
    const float floor_rotation_angle = 90.0f;
    const glm::vec3 floor_rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);

    // Ceiling.
    glm::vec3 ceiling_translation_vec;
    const float ceiling_rotation_angle = -90.0f;
    const glm::vec3 ceiling_rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);

    // Walls.
    std::vector<glm::vec3> wall_translation_vecs;
    const std::vector<float> wall_rotation_angles = {
        180.0f,
        0.0f,
        90.0f,
        90.0f,
    };
    const std::vector<glm::vec3> wall_rotation_axes = {
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
    };

    unsigned int depth_map;
    bool depth_map_set = false;

    // For debugging within render loop.
    bool first_loop = true;
    bool second_loop = false;
};

void Room::init()
{
    /*
     * Compute transformation matrices.
     */
    // Floor.
    floor_translation_vec = position;

    // Ceiling.
    ceiling_translation_vec = position + glm::vec3(0.0, scale_factor / 2, 0.0);

    // Walls.
    wall_translation_vecs = {
        glm::vec3(0.0, scale_factor / 4.0, -scale_factor / 2.0),
        glm::vec3(0.0, scale_factor / 4.0, scale_factor / 2.0),
        glm::vec3(-scale_factor / 2.0, scale_factor / 4.0, 0.0),
        glm::vec3(scale_factor / 2.0, scale_factor / 4.0, 0.0),
    };

    /*
     * Set up OpenGL buffers and other data.
     */
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * floor_vertices.size(), floor_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * square_indices.size(), square_indices.data(), GL_STATIC_DRAW);

    // Vertex positions.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // Vertex normals.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // Vertex textures coordinates.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // Load textures.
    floor_diffuse_texture = load_texture_from_file(floor_diffuse_texture_path);
    floor_specular_texture = load_texture_from_file(floor_specular_texture_path);
    ceiling_diffuse_texture = load_texture_from_file(ceiling_diffuse_texture_path);
    ceiling_specular_texture = load_texture_from_file(ceiling_specular_texture_path);
    wall_diffuse_texture = load_texture_from_file(wall_diffuse_texture_path);
    wall_specular_texture = load_texture_from_file(wall_specular_texture_path);
}

void Room::deinit()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Room::draw(Shader* shader)
{
    if (first_loop)
        logger.log(LogLevel::debug, "Room::draw (first loop)\n");
    else if (second_loop)
        logger.log(LogLevel::debug, "Room::draw (second loop)\n");

    if (!shader)
    {
        logger.log(LogLevel::warning, "Room::draw: shader is null");
        return;
    }

    shader->use();

    // Set shader attributes.
    shader->set_int("material.texture_diffuse1", 0);
    shader->set_int("material.texture_specular1", 1);
    shader->set_int("shadow_map", 2);
    shader->set_float("material.shininess", 16.0f);

    // Set depth map for room if possible.
    if (depth_map_set)
    {
        if (second_loop)
            logger.log(LogLevel::debug, "Room::draw (second loop): set depth map\n");

        if (second_loop)
            logger.log(LogLevel::debug, "Room::draw (second loop): depth_map = ", depth_map, '\n');
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depth_map);
        shader->set_int("shadow_map", 2);
    }

    /*
     * Set light attributes.
     */
    if (!sl)
    {
        logger.log(LogLevel::warning, "Mesh::draw: No scene lighting present\n");
        return;
    }

    // Directional light properties.
    if (sl && sl->dir)
    {
        shader->set_vec3("dir_light.direction", sl->dir->direction);

        shader->set_vec3("dir_light.ambient", sl->dir->ambient);
        shader->set_vec3("dir_light.diffuse", sl->dir->diffuse);
        shader->set_vec3("dir_light.specular", sl->dir->specular);
    }
    else
    {
        logger.log(LogLevel::debug, "Room:draw: No directional light present in the scene\n");
    }

    // Point light properties.
    for (std::size_t i = 0; i < sl->points.size(); i++)
    {
        std::string attr_prefix{"point_lights[" + std::to_string(i) + "]."};

        if (sl && sl->points[i])
        {
            shader->set_vec3(attr_prefix + "position", sl->points[i]->position);
            shader->set_vec3(attr_prefix + "ambient", sl->points[i]->ambient);
            shader->set_vec3(attr_prefix + "diffuse", sl->points[i]->color * sl->points[i]->diffuse);
            shader->set_vec3(attr_prefix + "specular", sl->points[i]->color * sl->points[i]->specular);
            shader->set_float(attr_prefix + "constant", sl->points[i]->constant);
            shader->set_float(attr_prefix + "linear", sl->points[i]->linear);
            shader->set_float(attr_prefix + "quadratic", sl->points[i]->quadratic);

        }
        else
        {
            logger.log(LogLevel::debug, "Room:draw: No point light present in the scene\n");
        }
    }

    // Spotlight properties.
    if (sl && sl->spot)
    {
        shader->set_vec3("spotlight.position", sl->spot->position);
        shader->set_vec3("spotlight.direction", sl->spot->direction);

        shader->set_float("spotlight.inner_cutoff", glm::cos(glm::radians(sl->spot->inner_cutoff)));
        shader->set_float("spotlight.outer_cutoff", glm::cos(glm::radians(sl->spot->outer_cutoff)));

        shader->set_vec3("spotlight.ambient", sl->spot->ambient);
        shader->set_vec3("spotlight.diffuse", sl->spot->diffuse);
        shader->set_vec3("spotlight.specular", sl->spot->specular);

        shader->set_float("spotlight.constant", sl->spot->constant);
        shader->set_float("spotlight.linear", sl->spot->linear);
        shader->set_float("spotlight.quadratic", sl->spot->quadratic);
    }
    else
    {
        logger.log(LogLevel::debug, "Room:draw: No spotlight present in the scene\n");
    }

    /*
     * Initialize model matrix.
     */
    glm::mat4 model = glm::mat4(1.0f);

    /*
     * Draw floor.
     */
    // Bind vertex buffers.
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * floor_vertices.size(), floor_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * square_indices.size(), square_indices.data(), GL_STATIC_DRAW);

    // Set model matrix.
    model = glm::mat4(1.0f);
    model = glm::translate(model, floor_translation_vec);
    model = glm::rotate(model, glm::radians(floor_rotation_angle), floor_rotation_axis);
    model = glm::scale(model, glm::vec3(scale_factor));
    shader->set_mat4fv("model", model);

    // Set textures.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floor_diffuse_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, floor_specular_texture);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (second_loop)
        logger.log(LogLevel::debug, "Room::draw (second loop): call floor shader\n");
    glDrawElements(GL_TRIANGLES, square_indices.size(), GL_UNSIGNED_INT, 0);

    /*
     * Draw ceiling.
     */
    // Bind vertex buffers.
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * floor_vertices.size(), floor_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * square_indices.size(), square_indices.data(), GL_STATIC_DRAW);

    // Set model matrix.
    model = glm::mat4(1.0f);
    model = glm::translate(model, ceiling_translation_vec);
    model = glm::rotate(model, glm::radians(ceiling_rotation_angle), ceiling_rotation_axis);
    model = glm::scale(model, glm::vec3(scale_factor));
    shader->set_mat4fv("model", model);

    // Set textures.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ceiling_diffuse_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ceiling_specular_texture);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawElements(GL_TRIANGLES, square_indices.size(), GL_UNSIGNED_INT, 0);

    /*
     * Draw walls.
     */
    // Bind vertex buffers.
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * wall_vertices.size(), wall_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * square_indices.size(), square_indices.data(), GL_STATIC_DRAW);

    // Set model matrix.
    assert(wall_translation_vecs.size() == wall_rotation_angles.size());
    assert(wall_translation_vecs.size() == wall_rotation_axes.size());
    for (std::size_t i = 0; i < wall_translation_vecs.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, wall_translation_vecs[i]);
        model = glm::rotate(model, glm::radians(wall_rotation_angles[i]), wall_rotation_axes[i]);

        // Add rotation for one wall.
        if (i == 2)
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, glm::vec3(scale_factor));
        shader->set_mat4fv("model", model);

        // Set textures.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wall_diffuse_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wall_specular_texture);

        glDrawElements(GL_TRIANGLES, square_indices.size(), GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    if (second_loop)
        second_loop = false;
    if (first_loop)
        second_loop = true;
    first_loop = false;
}

void Room::set_depth_map(unsigned int texture_id)
{
    depth_map = texture_id;
    depth_map_set = true;
}

#endif /* ROOM_HPP */
