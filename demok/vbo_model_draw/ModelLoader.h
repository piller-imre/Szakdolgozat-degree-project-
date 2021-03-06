#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <glew.h>
#include <SOIL.h>
#include <iostream>
#include <vector>

#include <string.h>

using namespace std;

#define TRUE 1
#define FALSE 0

#define INVALID_VERTEX_INDEX 0

struct TokenArray
{
	char** tokens;
	int n_tokens;
};

struct Vertex
{
	double x;
	double y;
	double z;
};

struct TextureVertex
{
	double u;
	double v;
};

struct FacePoint
{
	int vertex_index;
	int texture_index;
	int normal_index;
};

struct Triangle
{
	struct FacePoint points[3];
};

struct Quad
{
	struct FacePoint points[4];
};

struct Model
{
	int n_vertices;
	int n_texture_vertices;
	int n_normals;
	int n_triangles;
	int n_quads;
	struct Vertex* vertices;
	struct TextureVertex* texture_vertices;
	struct Vertex* normals;
	struct Triangle* triangles;
	struct Quad* quads;
};

struct VboVertex
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat u;
	GLfloat v;
};

/**
		*A h�romsz�gek adatainak a VBO-nak megfelel� strut�r�ba konvert�l�sa
*/
std::vector<VboVertex> convert_triangles_to_vbo(const Model& model);

/**
		*A n�gysz�gek adatainak a VBO-nak megfelel� strut�r�ba konvert�l�sa
*/
std::vector<VboVertex> convert_quads_to_vbo(const Model& model);

class Model_loader
{
public:
	Model_loader();

	~Model_loader();

	bool Load(const char* model_name, double size_x, double size_y, double size_z, const char* texture_name);

protected:
	Model model;
	GLuint texture;
	GLuint m_vbo_id;
	std::vector<VboVertex> m_vbo_vertex_triangles;
	std::vector<VboVertex> m_vbo_vertex_quads;
	std::vector<VboVertex> m_vbo_vertex_all;
	/**
			*Tokenek sz�mol�sa
	*/
	int count_tokens(const char* text);

	/**
			*.obj modellek bet�lt�se
	*/
	int load_model(const char* filename, struct Model* model);

	GLuint get_texture();

	void load_texture(const char* texture_name);

	/**
			*Bet�lt�tt modell m�retez�se
	*/
	void scale_model(struct Model* model, double sx, double sy, double sz);

	/**
			*Token hossz�nak sz�m�t�sa
	*/
	int calc_token_length(const char* text, int start_index);

	/**
			*M�solatot k�sz�t a tokenr�l
	*/
	char* copy_token(const char* text, int offset, int length);

	/**
			*Token beilleszt�se a strut�r�ba
	*/
	void insert_token(const char* token, struct TokenArray* token_array);

	/**
			*Tokenek kibont�sa sz�vegb�l
	*/
	void extract_tokens(const char* text, struct TokenArray* token_array);

	/**
			*Mem�ria felszabad�t�sa, amit a tokenek foglaltak
	*/
	void free_tokens(struct TokenArray* token_array);

	/**
			*Inform�ci�k ki�rat�sa a bet�lt�tt modellr�l
	*/
	void print_model_info(const struct Model* model);

	/**
			*Mem�ria felszabad�t�sa, amit a modellek foglaltak
	*/
	void free_model(struct Model* model);

	/**
			* Count the elements in the model and set counts in the structure.
	*/
	void count_elements(FILE* file, struct Model* model);

	/**
			* Read the elements of the model and fill the structure with values.
	*/
	void read_elements(FILE* file, struct Model* model);

	/**
			* Initializes model counters to zero.
	*/
	void init_model_counters(struct Model* model);

	/**
			* Clear the comment from the end of the line.
	*/
	void clear_comment(char* line);

	/**
			* Determine the type of the line and increment the appropriate counter.
	*/
	void count_element_in_line(const char* line, struct Model* model);

	/**
			* Read the given data from the line.
	*/
	void read_element_from_line(const char* line, struct Model* model);

	/**
			* Allocate memory for the model.
			*
			* The OBJ file format starts indices from 1. From this reason,
			* the vertices, texture vertices and normals have a blank 0 element.
	*/
	void create_arrays(struct Model* model);

	/**
			* Read vertex data.
	*/
	void read_vertex(const struct TokenArray* token_array, struct Vertex* vertex);

	/**
			* Read texture vertex data.
	*/
	void read_texture_vertex(const struct TokenArray* token_array, struct TextureVertex* texture_vertex);

	/**
			* Read normal vector data.
	*/
	void read_normal(const struct TokenArray* token_array, struct Vertex* normal);

	/**
			* Read triangle data.
	*/
	void read_triangle(const struct TokenArray* token_array, struct Triangle* triangle);

	/**
			* Read quad data.
	*/
	void read_quad(const struct TokenArray* token_array, struct Quad* quad);

	/**
			* Read face point data.
	*/
	void read_face_point(const char* text, struct FacePoint* face_point);

	/**
			* Count the delimiters in the face token.
	*/
	int count_face_delimiters(const char* text);

	/**
			* Read the next index from the string.
			*
			* It also calculate the length of the number and returns in length.
	*/
	int read_next_index(const char* text, int* length);

	/**
			* Check that the next number is a number digit.
	*/
	int is_digit(char c);

	/**
			* Check that the indices in the triangle are valid.
	*/
	int is_valid_triangle(const struct Triangle* triangle, const struct Model* model);

	/**
			* Check that the indices in the quad are valid.
	*/
	int is_valid_quad(const struct Quad* quad, const struct Model* model);

	/**
			* Print the bounding box of the model.
	*/
	void print_bounding_box(const struct Model* model);

};

#endif // MODEL_H

