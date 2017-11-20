#include "ModelLoader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 1024

Model_loader::Model_loader()
{
	memset(&model, 0, sizeof(model));
	texture = 0;
	m_vbo_id = 0;
}

Model_loader::~Model_loader()
{
	if (m_vbo_id != 0)
	{
		glDeleteBuffers(1, &m_vbo_id);
	}
}


bool Model_loader::Load(const char* model_name, double size_x, double size_y, double size_z, const char* texture_name)
{
	if (m_vbo_id == 0)
	{
		glGenBuffers(1, &m_vbo_id);
	}
	else return false;
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
	load_model(model_name, &model);
	load_texture(texture_name);
	print_model_info(&model);
	scale_model(&model, size_x, size_y, size_z);
	m_vbo_vertex_triangles = convert_triangles_to_vbo(model);
	m_vbo_vertex_quads = convert_quads_to_vbo(model);
	m_vbo_vertex_all = m_vbo_vertex_triangles;
	m_vbo_vertex_all.insert(m_vbo_vertex_all.end(), m_vbo_vertex_quads.begin(), m_vbo_vertex_quads.end());
	glBufferData(GL_ARRAY_BUFFER, sizeof(VboVertex) * m_vbo_vertex_all.size(), m_vbo_vertex_all.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

GLuint Model_loader::get_texture()
{
	return texture;
}

void Model_loader::load_texture(const char* texture_name)
{
	texture = SOIL_load_OGL_texture
	(
		texture_name,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		//SOIL_FLAG_MIPMAPS | SOIL_FLAG_POWER_OF_TWO
		0
	);

	if (texture == 0)
	{
		cout << "ERROR: '" << texture_name << "' could not loaded (texture missing?)\n";
	}
	else
	{
		cout << "The '" << texture_name << "' has successfully loaded!" << endl;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

std::vector<VboVertex> convert_triangles_to_vbo(const Model& model)
{
	std::vector<VboVertex> vertices;
	VboVertex vbo_vertex;
	for (int i = 0; i < model.n_triangles; ++i) {
		Triangle* triangle = &model.triangles[i];
		for (int k = 0; k < 3; ++k) {

			int vertex_index = triangle->points[k].vertex_index;
			vbo_vertex.x = model.vertices[vertex_index].x;
			vbo_vertex.y = model.vertices[vertex_index].y;
			vbo_vertex.z = model.vertices[vertex_index].z;
			vbo_vertex.r = 1.0f;
			vbo_vertex.g = 1.0f;
			vbo_vertex.b = 1.0f;

			int texture_index = triangle->points[k].texture_index;
			GLfloat u = model.texture_vertices[texture_index].u;
			GLfloat v = model.texture_vertices[texture_index].v;
			v = 1 - v;
			vbo_vertex.u = u;
			vbo_vertex.v = v;

			vertices.push_back(vbo_vertex);
		}
	}
	return vertices;
}

std::vector<VboVertex> convert_quads_to_vbo(const Model& model)
{
	std::vector<VboVertex> vertices;
	VboVertex vbo_vertex;
	for (int i = 0; i < model.n_quads; ++i) {
		Quad* quad = &model.quads[i];
		for (int k = 0; k < 4; ++k) {

			int vertex_index = quad->points[k].vertex_index;
			vbo_vertex.x = model.vertices[vertex_index].x;
			vbo_vertex.y = model.vertices[vertex_index].y;
			vbo_vertex.z = model.vertices[vertex_index].z;
			vbo_vertex.r = 1.0f;
			vbo_vertex.g = 1.0f;
			vbo_vertex.b = 1.0f;

			int texture_index = quad->points[k].texture_index;
			GLfloat u = model.texture_vertices[texture_index].u;
			GLfloat v = model.texture_vertices[texture_index].v;
			v = 1 - v;
			vbo_vertex.u = u;
			vbo_vertex.v = v;

			vertices.push_back(vbo_vertex);
		}
	}
	return vertices;
}

int Model_loader::count_tokens(const char* text)
{
	int i = 0;
	int is_token = FALSE;
	int count = 0;

	while (text[i] != 0) {
		if (is_token == FALSE && text[i] != ' ') {
			++count;
			is_token = TRUE;
		}
		else if (is_token == TRUE && text[i] == ' ') {
			is_token = FALSE;
		}
		++i;
	}

	return count;
}

void Model_loader::extract_tokens(const char* text, struct TokenArray* token_array)
{
	int n_tokens, token_length;
	char* token;
	int i;

	n_tokens = count_tokens(text);

	token_array->tokens = (char**)malloc(n_tokens * sizeof(char*));
	token_array->n_tokens = 0;

	i = 0;
	while (text[i] != 0) {
		if (text[i] != ' ') {
			token_length = calc_token_length(text, i);
			token = copy_token(text, i, token_length);
			insert_token(token, token_array);
			i += token_length;
		}
		else {
			++i;
		}
	}
}

char* Model_loader::copy_token(const char* text, int offset, int length)
{
	char* token;
	int i;

	token = (char*)malloc((length + 1) * sizeof(char));
	for (i = 0; i < length; ++i) {
		token[i] = text[offset + i];
	}
	token[i] = 0;

	return token;
}

void Model_loader::insert_token(const char* token, struct TokenArray* token_array)
{
	token_array->tokens[token_array->n_tokens] = (char*)token;
	++token_array->n_tokens;
}

int Model_loader::calc_token_length(const char* text, int start_index)
{
	int end_index, length;

	end_index = start_index;
	while (text[end_index] != 0 && text[end_index] != ' ') {
		++end_index;
	}
	length = end_index - start_index;

	return length;
}

void Model_loader::free_tokens(struct TokenArray* token_array)
{
	int i;

	for (i = 0; i < token_array->n_tokens; ++i) {
		free(token_array->tokens[i]);
	}
	free(token_array->tokens);
}

int Model_loader::load_model(const char* filename, struct Model* model)
{
	FILE* obj_file = fopen(filename, "r");
	printf("Load model '%s' ...\n", filename);
	if (obj_file == NULL) {
		printf("ERROR: Unable to open '%s' file!\n", filename);
		return FALSE;
	}
	printf("Count ..\n");
	count_elements(obj_file, model);
	printf("Create ..\n");
	create_arrays(model);
	printf("Read ..\n");
	read_elements(obj_file, model);
	if (obj_file != NULL)
	{
		cout << "The '" << filename << "' has loaded!" << endl;
	}

	return TRUE;
}

void Model_loader::print_model_info(const struct Model* model)
{
	printf("Vertices: %d\n", model->n_vertices);
	printf("Texture vertices: %d\n", model->n_texture_vertices);
	printf("Normals: %d\n", model->n_normals);
	printf("Triangles: %d\n", model->n_triangles);
	printf("Quads: %d\n\n", model->n_quads);
}

void Model_loader::free_model(struct Model* model)
{
	free(model->vertices);
	free(model->texture_vertices);
	free(model->normals);
	free(model->triangles);
	free(model->quads);
}

void Model_loader::count_elements(FILE* file, struct Model* model)
{
	char line[LINE_BUFFER_SIZE];

	init_model_counters(model);
	while (fgets(line, LINE_BUFFER_SIZE, file) != NULL) {
		clear_comment(line);
		count_element_in_line(line, model);
	}
}

void Model_loader::read_elements(FILE* file, struct Model* model)
{
	char line[LINE_BUFFER_SIZE];

	init_model_counters(model);
	model->n_vertices = 1;
	model->n_texture_vertices = 1;
	model->n_normals = 1;

	fseek(file, 0, SEEK_SET);
	while (fgets(line, LINE_BUFFER_SIZE, file) != NULL) {
		clear_comment(line);
		read_element_from_line(line, model);
	}
}

void Model_loader::init_model_counters(struct Model* model)
{
	model->n_vertices = 0;
	model->n_texture_vertices = 0;
	model->n_normals = 0;
	model->n_triangles = 0;
	model->n_quads = 0;
}

void Model_loader::clear_comment(char* line)
{
	int i = 0;
	while (line[i] != 0 && line[i] != '#') {
		++i;
	}
	if (line[i] == '#') {
		while (line[i] != 0) {
			line[i] = ' ';
			++i;
		}
	}
}

void Model_loader::count_element_in_line(const char* line, struct Model* model)
{
	struct TokenArray token_array;
	char* first_token;

	extract_tokens(line, &token_array);

	if (token_array.n_tokens > 0) {
		first_token = token_array.tokens[0];
		if (strcmp(first_token, "v") == 0) {
			++model->n_vertices;
		}
		else if (strcmp(first_token, "vt") == 0) {
			++model->n_texture_vertices;
		}
		else if (strcmp(first_token, "vn") == 0) {
			++model->n_normals;
		}
		else if (strcmp(first_token, "f") == 0) {
			if (token_array.n_tokens == 1 + 3) {
				++model->n_triangles;
			}
			else if (token_array.n_tokens == 1 + 4) {
				++model->n_quads;
			}
			else {
				printf("WARN: Invalid number of face elements! %d\n", token_array.n_tokens);
			}
		}
	}

	free_tokens(&token_array);
}

void Model_loader::read_element_from_line(const char* line, struct Model* model)
{
	struct TokenArray token_array;
	char* first_token;
	struct Triangle* triangle;
	struct Quad* quad;

	extract_tokens(line, &token_array);

	if (token_array.n_tokens > 0) {
		first_token = token_array.tokens[0];
		if (strcmp(first_token, "v") == 0) {
			read_vertex(&token_array, &(model->vertices[model->n_vertices]));
			++model->n_vertices;
		}
		else if (strcmp(first_token, "vt") == 0) {
			read_texture_vertex(&token_array, &(model->texture_vertices[model->n_texture_vertices]));
			++model->n_texture_vertices;
		}
		else if (strcmp(first_token, "vn") == 0) {
			read_normal(&token_array, &(model->normals[model->n_normals]));
			++model->n_normals;
		}
		else if (strcmp(first_token, "f") == 0) {
			if (token_array.n_tokens == 1 + 3) {
				triangle = &(model->triangles[model->n_triangles]);
				read_triangle(&token_array, triangle);
				if (is_valid_triangle(triangle, model) == FALSE) {
					printf("line: '%s'\n", line);
				}
				++model->n_triangles;
			}
			else if (token_array.n_tokens == 1 + 4) {
				quad = &(model->quads[model->n_quads]);
				read_quad(&token_array, quad);
				if (is_valid_quad(quad, model) == FALSE) {
					printf("line: '%s'\n", line);
				}
				++model->n_quads;
			}
		}
	}

	free_tokens(&token_array);
}

void Model_loader::create_arrays(struct Model* model)
{
	model->vertices = (struct Vertex*)malloc((model->n_vertices + 1) * sizeof(struct Vertex));
	model->texture_vertices = (struct TextureVertex*)malloc((model->n_texture_vertices + 1) * sizeof(struct TextureVertex));
	model->normals = (struct Vertex*)malloc((model->n_normals + 1) * sizeof(struct Vertex));
	model->triangles = (struct Triangle*)malloc(model->n_triangles * sizeof(struct Triangle));
	model->quads = (struct Quad*)malloc(model->n_quads * sizeof(struct Quad));
}

void Model_loader::read_vertex(const struct TokenArray* token_array, struct Vertex* vertex)
{
	vertex->x = atof(token_array->tokens[1]);
	vertex->y = atof(token_array->tokens[2]);
	vertex->z = atof(token_array->tokens[3]);
}

void Model_loader::read_texture_vertex(const struct TokenArray* token_array, struct TextureVertex* texture_vertex)
{
	texture_vertex->u = atof(token_array->tokens[1]);
	texture_vertex->v = atof(token_array->tokens[2]);
}

void Model_loader::read_normal(const struct TokenArray* token_array, struct Vertex* normal)
{
	normal->x = atof(token_array->tokens[1]);
	normal->y = atof(token_array->tokens[2]);
	normal->z = atof(token_array->tokens[3]);
}

void Model_loader::read_triangle(const struct TokenArray* token_array, struct Triangle* triangle)
{
	int i;

	for (i = 0; i < 3; ++i) {
		read_face_point(token_array->tokens[i + 1], &triangle->points[i]);
	}
}

void Model_loader::read_quad(const struct TokenArray* token_array, struct Quad* quad)
{
	int i;

	for (i = 0; i < 4; ++i) {
		read_face_point(token_array->tokens[i + 1], &quad->points[i]);
	}
}

void Model_loader::read_face_point(const char* text, struct FacePoint* face_point)
{
	int delimiter_count;
	const char* token;
	int length;

	token = text;
	delimiter_count = count_face_delimiters(text);

	if (delimiter_count == 0) {
		face_point->vertex_index = read_next_index(token, &length);
		face_point->texture_index = INVALID_VERTEX_INDEX;
		face_point->normal_index = INVALID_VERTEX_INDEX;
	}
	else if (delimiter_count == 1) {
		face_point->vertex_index = read_next_index(token, &length);
		token += length;
		face_point->texture_index = read_next_index(token, &length);
		face_point->normal_index = INVALID_VERTEX_INDEX;
	}
	else if (delimiter_count == 2) {
		face_point->vertex_index = read_next_index(token, &length);
		token += length;
		face_point->texture_index = read_next_index(token, &length);
		token += length;
		face_point->normal_index = read_next_index(token, &length);
	}
	else {
		printf("ERROR: Invalid face token! '%s'", text);
	}
}

int Model_loader::count_face_delimiters(const char* text)
{
	int count, i;

	count = 0;
	i = 0;
	while (text[i] != 0) {
		if (text[i] == '/') {
			++count;
		}
		++i;
	}

	return count;
}

int Model_loader::read_next_index(const char* text, int* length)
{
	int i, j, index;
	char buffer[32];

	i = 0;
	while (text[i] != 0 && is_digit(text[i]) == FALSE) {
		++i;
	}

	if (text[i] == 0) {
		return INVALID_VERTEX_INDEX;
	}

	j = 0;
	while (text[i] != 0 && is_digit(text[i]) == TRUE) {
		buffer[j] = text[i];
		++i;
		++j;
	}
	buffer[j] = 0;

	index = atoi(buffer);
	*length = i;

	return index;
}

int Model_loader::is_digit(char c)
{
	if (c >= '0' && c <= '9') {
		return TRUE;
	}
	return FALSE;
}

int Model_loader::is_valid_triangle(const struct Triangle* triangle, const struct Model* model)
{
	int k;

	for (k = 0; k < 3; ++k) {
		if (triangle->points[k].vertex_index >= model->n_vertices) {
			printf("ERROR: Invalid vertex index in a triangle!\n");
			return FALSE;
		}
		if (triangle->points[k].texture_index >= model->n_texture_vertices) {
			printf("ERROR: Invalid texture vertex index in a triangle!\n");
			return FALSE;
		}
		if (triangle->points[k].normal_index >= model->n_normals) {
			printf("ERROR: Invalid normal index in a triangle!\n");
			return FALSE;
		}
	}
	return TRUE;
}

int Model_loader::is_valid_quad(const struct Quad* quad, const struct Model* model)
{
	int k;
	int vertex_index, texture_index, normal_index;

	for (k = 0; k < 4; ++k) {
		vertex_index = quad->points[k].vertex_index;
		texture_index = quad->points[k].texture_index;
		normal_index = quad->points[k].normal_index;
		if (vertex_index < 0 || vertex_index >= model->n_vertices) {
			printf("ERROR: Invalid vertex index in a quad!\n");
			return FALSE;
		}
		if (texture_index < 0 || texture_index >= model->n_texture_vertices) {
			printf("ERROR: Invalid texture vertex index in a quad!\n");
			return FALSE;
		}
		if (normal_index < 0 || normal_index >= model->n_normals) {
			printf("ERROR: Invalid normal index in a quad!");
			return FALSE;
		}
	}
	return TRUE;
}

void Model_loader::print_bounding_box(const struct Model* model)
{
	int i;
	double x, y, z;
	double min_x, max_x, min_y, max_y, min_z, max_z;

	if (model->n_vertices == 0) {
		return;
	}

	min_x = model->vertices[0].x;
	max_x = model->vertices[0].x;
	min_y = model->vertices[0].y;
	max_y = model->vertices[0].y;
	min_z = model->vertices[0].z;
	max_z = model->vertices[0].z;

	for (i = 0; i < model->n_vertices; ++i) {
		x = model->vertices[i].x;
		y = model->vertices[i].y;
		z = model->vertices[i].z;
		if (x < min_x) {
			min_x = x;
		}
		else if (x > max_x) {
			max_x = x;
		}
		if (y < min_y) {
			min_y = y;
		}
		else if (y > max_y) {
			max_y = y;
		}
		if (z < min_z) {
			min_z = z;
		}
		else if (z > max_z) {
			max_z = z;
		}
	}

	printf("Bounding box:\n");
	printf("x in [%lf, %lf]\n", min_x, max_x);
	printf("y in [%lf, %lf]\n", min_y, max_y);
	printf("z in [%lf, %lf]\n", min_z, max_z);
}

void Model_loader::scale_model(struct Model* model, double sx, double sy, double sz)
{
	int i;

	for (i = 0; i < model->n_vertices; ++i) {
		model->vertices[i].x *= sx;
		model->vertices[i].y *= sy;
		model->vertices[i].z *= sz;
	}
}