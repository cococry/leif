#include "leif.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <stb_truetype.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef LF_GLFW 
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif /* ifdef */

#define LF_TRACE(...) { printf("Leif: [TRACE]: "); printf(__VA_ARGS__); printf("\n"); } 
#ifdef LF_DEBUG
#define LF_DEBUG(...) { printf("Leif: [DEBUG]: "); printf(__VA_ARGS__); printf("\n"); } 
#else
#define LF_DEBUG(...) 
#endif // LF_DEBUG 
#define LF_INFO(...) { printf("Leif: [INFO]: "); printf(__VA_ARGS__); printf("\n"); } 
#define LF_WARN(...) { printf("Leif: [WARN]: "); printf(__VA_ARGS__); printf("\n"); } 
#define LF_ERROR(...) { printf("[LEIF ERROR]: "); printf(__VA_ARGS__); printf("\n"); } 

#ifdef _MSC_VER 
#define D_BREAK __debugbreak
#elif defined(__clang__) || defined(__GNUC__)
#define D_BREAK __builtin_trap
#else 
#define D_BREAK
#endif 

#ifdef _DEBUG 
#define LF_ASSERT(cond, ...) { if(cond) {} else { printf("[LEIF]: Assertion failed: '"); printf(__VA_ARGS__); printf("' in file '%s' on line %i.\n", __FILE__, __LINE__); D_BREAK(); }}
#else 
#define LF_ASSERT(cond, ...)
#endif // _DEBUG


#ifdef LF_GLFW
#define MAX_KEYS GLFW_KEY_LAST
#define MAX_MOUSE_BUTTONS GLFW_MOUSE_BUTTON_LAST
#define KEY_CALLBACK_t GLFWkeyfun
#define MOUSE_BUTTON_CALLBACK_t GLFWmousebuttonfun
#define SCROLL_CALLBACK_t GLFWscrollfun
#define CURSOR_CALLBACK_t GLFWcursorposfun
#else 

#define MAX_KEYS 0
#define MAX_MOUSE_BUTTONS 0
#define KEY_CALLBACK_t void*
#define MOUSE_BUTTON_CALLBACK_t void*
#define SCROLL_CALLBACK_t void*
#define CURSOR_CALLBACK_t void*
#endif
#define MAX_VERT_COUNT_BATCH 10000
#define MAX_TEX_COUNT_BATCH 32
#define MAX_KEY_CALLBACKS 4
#define MAX_MOUSE_BTTUON_CALLBACKS 4
#define MAX_SCROLL_CALLBACKS 4
#define MAX_CURSOR_POS_CALLBACKS 4

// -- Struct Defines ---
typedef struct {
    uint32_t id;
} LfShader;

typedef struct {
    LfVec2f pos;
    LfVec4f color;
    LfVec2f texcoord;
    float tex_index;
} Vertex;

typedef struct {
    LfVec2i size, pos;    
} LfAABB;

typedef struct {
    LfAABB aabb;
    bool init, init_size;
    LfClickableItemState interact_state;
} LfDiv;

typedef struct {
    bool keys[MAX_KEYS];
    bool keys_changed[MAX_KEYS];
} LfKeyboard;

typedef struct {
    bool buttons_current[MAX_MOUSE_BUTTONS];
    bool buttons_last[MAX_MOUSE_BUTTONS];
    double xpos, ypos, xpos_last, ypos_last, xpos_delta, ypos_delta;
    bool first_mouse_press; 
    double xscroll_delta, yscroll_delta;
} LfMouse;
typedef struct {
    LfKeyboard keyboard;
    LfMouse mouse;
    KEY_CALLBACK_t key_cbs[MAX_KEY_CALLBACKS];
    MOUSE_BUTTON_CALLBACK_t mouse_button_cbs[MAX_MOUSE_BTTUON_CALLBACKS];
    SCROLL_CALLBACK_t scroll_cbs[MAX_SCROLL_CALLBACKS];
    CURSOR_CALLBACK_t cursor_pos_cbs[MAX_CURSOR_POS_CALLBACKS];

    uint32_t key_cb_count, mouse_button_cb_count, scroll_cb_count, cursor_pos_cb_count;
} InputState;

typedef struct {
    LfShader shader;
    uint32_t vao, vbo;
    uint32_t vert_count;
    Vertex verts[MAX_VERT_COUNT_BATCH];
    LfVec4f vert_pos[6];
    LfTexture textures[MAX_TEX_COUNT_BATCH];
    uint32_t tex_index, tex_count;
} RenderState;

typedef struct {
    bool init;
    uint32_t dsp_w, dsp_h;
    void* window_handle;

    RenderState render;
    InputState input;
    LfTheme theme;

    
    LfDiv current_div;
    int32_t current_line_height;
    LfVec2i pos_ptr;

} LfState;

static LfState state;


// --- Renderer ---
static uint32_t                 shader_create(GLenum type, const char* src);
static LfShader                 shader_prg_create(char* vert_src, char* frag_src);
static void                     shader_set_mat(LfShader prg, const char* name, LfMat4 mat); 

static void                     renderer_init();

static LfTexture                tex_create(const char* filepath, bool flip, LfTextureFiltering filter);

// --- Linear Algebra - Math ---
static LfMat4                   mat_identity(); 
static LfMat4                   orth_mat(float left, float right, float bottom, float top);
static LfMat4                   translate_mat(LfMat4 mat, LfVec3f translation);
static LfMat4                   scale_mat(LfMat4 mat, LfVec3f scale);
static LfMat4                   mat_mul(LfMat4 m1, LfMat4 m2);
static LfVec4f                  vec4_mul_mat(LfVec4f vec, LfMat4 mat);

static void                     mat_set_row_val(LfMat4* mat, uint32_t row_index, uint32_t vec_index, float val);
static float                    mat_get_row_val(LfMat4 mat, uint32_t row_index, uint32_t vec_index);

static void                     mat_set_row(LfMat4* mat, uint32_t row_index, LfVec4f val);
static LfVec4f                  mat_get_row(LfMat4 mat, uint32_t row_index);

static LfVec4f                  vec4f_create(float x, float y, float z, float w);
static LfVec3f                  vec3f_create(float x, float y, float z);
static LfVec2f                  vec2f_create(float x, float y);

static LfVec4f                  vec4f_add(LfVec4f a, LfVec4f b);
static LfVec4f                  vec4f_mul_scaler(LfVec4f v, float k);
static LfVec4f                  vec4f_mul(LfVec4f v1, LfVec4f v2);

static bool                     point_intersects_aabb(LfVec2f p, LfAABB aabb);
static bool                     aabb_intersects_aabb(LfAABB a, LfAABB b);

// --- UI ---
static LfClickableItemState     clickable_item(LfVec2i pos, LfVec2i size, LfUIElementProps props);
static LfTextProps              lf_text_render(LfVec2i pos, const char* str, LfFont font, LfUIElementProps props, float wrap_point, bool no_wrap, bool no_render);
static void                     lf_image_render(LfVec2i pos, LfVec4f color, LfTexture tex);

// --- Input ---
#ifdef LF_GLFW
static void                     glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void                     glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods); 
static void                     glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void                     glfw_cursor_callback(GLFWwindow* window, double xpos, double ypos);
#endif

// --- Static Functions --- 
uint32_t shader_create(GLenum type, const char* src) {
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int32_t compiled; 
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled) {
        LF_ERROR("Failed to compile %s shader.", type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        LF_INFO("%s", info);
        glDeleteShader(shader);
    }
    return shader;
}

LfShader shader_prg_create(char* vert_src, char* frag_src) {
    uint32_t vertex_shader = shader_create(GL_VERTEX_SHADER, vert_src);
    uint32_t fragment_shader = shader_create(GL_FRAGMENT_SHADER, frag_src);

    LfShader prg;

    prg.id = glCreateProgram();
    glAttachShader(prg.id, vertex_shader);
    glAttachShader(prg.id, fragment_shader);
    glLinkProgram(prg.id);

    int32_t linked;
    glGetProgramiv(prg.id, GL_LINK_STATUS, &linked);

    if(!linked) {
        LF_ERROR("Failed to link shader program.");
        char info[512];
        glGetProgramInfoLog(prg.id, 512, NULL, info);
        LF_INFO("%s", info);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(prg.id);
        return prg;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return prg;
}

static LfState state;

void renderer_init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    state.render.vert_count = 0;

    glCreateVertexArrays(1, &state.render.vao);
    glBindVertexArray(state.render.vao);
    
    glCreateBuffers(1, &state.render.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, state.render.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERT_COUNT_BATCH, NULL, 
        GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(intptr_t*)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(intptr_t*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(intptr_t*)(sizeof(float) * 8));
    glEnableVertexAttribArray(3);

    char* vert_src = 
        "#version 460 core\n"
        "layout (location = 0) in vec2 a_pos;\n"
        "layout (location = 1) in vec4 a_color;\n"
        "layout (location = 2) in vec2 a_texcoord;\n"
        "layout (location = 3) in float a_tex_index;\n"
        "uniform mat4 u_proj;\n"
        "out vec4 v_color;\n"
        "out vec2 v_texcoord;\n"
        "out float v_tex_index;\n"
        "void main() {\n"
            "v_color = a_color;\n"
            "v_texcoord = a_texcoord;\n"
            "v_tex_index = a_tex_index;\n"
            "gl_Position = u_proj * vec4(a_pos.x, a_pos.y, 0.0f, 1.0);\n"
        "}\n";

    char* frag_src = 
        "#version 460 core\n"
        "out vec4 o_color;\n"
        "in vec4 v_color;\n"
        "in vec2 v_texcoord;\n"
        "in float v_tex_index;\n"
        "uniform sampler2D u_textures[32];\n"
        "void main() {\n"
        "   if(v_tex_index == -1) {\n"
        "     o_color = v_color;\n"
        "   } else {\n"
        "     o_color = texture(u_textures[int(v_tex_index)], v_texcoord) * v_color;\n"
        "   }\n"
        "}\n";

    state.render.shader = shader_prg_create(vert_src, frag_src);

    state.render.vert_pos[0] = vec4f_create(-0.5f, -0.5f, 0.0f, 1.0f);
    state.render.vert_pos[1] = vec4f_create(-0.5f,  0.5f, 0.0f, 1.0f);
    state.render.vert_pos[2] = vec4f_create( 0.5f,  0.5f, 0.0f, 1.0f);

    state.render.vert_pos[3] = vec4f_create(-0.5f, -0.5f, 0.0f, 1.0f);
    state.render.vert_pos[4] = vec4f_create(0.5f,  0.5f, 0.0f, 1.0f);
    state.render.vert_pos[5] = vec4f_create(0.5f,  -0.5f, 0.0f, 1.0f);
    

    glUseProgram(state.render.shader.id);
    LfMat4 proj = orth_mat(0.0f, (float)state.dsp_w, (float)state.dsp_h, 0.0f);
    shader_set_mat(state.render.shader, "u_proj", proj);

    int32_t tex_slots[MAX_TEX_COUNT_BATCH];
    for(uint32_t i = 0; i < MAX_TEX_COUNT_BATCH; i++) 
        tex_slots[i] = i;

    glUniform1iv(glGetUniformLocation(state.render.shader.id, "u_textures"), MAX_TEX_COUNT_BATCH, tex_slots);
}
void shader_set_mat(LfShader prg, const char* name, LfMat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(prg.id, name), 1, GL_FALSE, (float*)&mat.values);
}

LfTexture lf_tex_create(const char* filepath, bool flip, LfTextureFiltering filter) {
    LfTexture tex;
    int32_t width, height, channels;
    stbi_set_flip_vertically_on_load(flip);
    stbi_uc* data = stbi_load(filepath, &width, &height, &channels, 0);

    if(!data) {
        LF_ERROR("Failed to load texture file at '%s'.", filepath);
        return tex;
    }
    GLenum internal_format = (channels == 4) ? GL_RGBA8 : GL_RGB8;
    GLenum data_format = (channels == 4) ? GL_RGBA : GL_RGB;
    
    LF_ASSERT(internal_format & data_format, "Texture file at '%s' is using an unsupported format.", filepath);

    glCreateTextures(GL_TEXTURE_2D, 1, &tex.id);
    glTextureStorage2D(tex.id, 1, internal_format, width, height);
    
    switch(filter) {
        case LF_TEX_FILTER_LINEAR:
            glTextureParameteri(tex.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(tex.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case LF_TEX_FILTER_NEAREST:
            glTextureParameteri(tex.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(tex.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
    }
    glTextureParameteri(tex.id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex.id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureSubImage2D(tex.id, 0, 0, 0, width, height, data_format, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);

    tex.width = width;
    tex.height = height;

    return tex;
}

LfFont lf_load_font(const char* filepath, uint32_t pixelsize, uint32_t tex_width, uint32_t tex_height, uint32_t num_glyphs, uint32_t line_gap_add) {
    LfFont font = {0};
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        LF_ERROR("Failed to open font file '%s'\n", filepath);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t* buffer = malloc(fileSize);
    fread(buffer, 1, fileSize, file);
    fclose(file); 
    font.font_info = malloc(sizeof(stbtt_fontinfo));
    stbtt_InitFont((stbtt_fontinfo*)font.font_info, buffer, stbtt_GetFontOffsetForIndex(buffer, 0));
    
    uint8_t buf[1<<20];
    uint8_t bitmap[tex_width * tex_height];
    fread(buf, 1, 1<<20, fopen(filepath, "rb"));
    font.cdata = malloc(sizeof(stbtt_bakedchar) * 256);
    font.tex_width = tex_width;
    font.tex_height = tex_height;
    font.line_gap_add = line_gap_add;
    font.font_size = pixelsize;
    stbtt_BakeFontBitmap(buf, 0, pixelsize, bitmap, tex_width, tex_height, 32, num_glyphs, (stbtt_bakedchar*)font.cdata);
    uint8_t bitmap_4bpp[tex_width * tex_height * 4];

    uint32_t bitmap_index = 0;
    for(uint32_t i = 0; i < (uint32_t)(tex_width * tex_height * 4); i++) {
        bitmap_4bpp[i] = bitmap[bitmap_index];
        if((i + 1) % 4 == 0) {
            bitmap_index++;
    int line_height = 16; // Adjust this value as needed
        }
    }

    glGenTextures(1, &font.bitmap.id);
    glBindTexture(GL_TEXTURE_2D, font.bitmap.id);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap_4bpp);
    // Generate mipmaps (optional)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Unbind the texture
    return font;
}

void lf_free_font(LfFont* font) {
    free(font->cdata);
    free(font->font_info);
}

LfMat4 mat_identity() {
    LfMat4 mat;
    mat.values[0] = 1;
    mat.values[1] = 0;
    mat.values[2] = 0;
    mat.values[3] = 0;

    mat.values[4] = 0;
    mat.values[5] = 1;
    mat.values[6] = 0;
    mat.values[7] = 0;

    mat.values[8] = 0;
    mat.values[9] = 0;
    mat.values[10] = 1;
    mat.values[11] = 0;
    
    mat.values[12] = 0;
    mat.values[13] = 0;
    mat.values[14] = 0;
    mat.values[15] = 1;
    return mat;
}

LfMat4 translate_mat(LfMat4 mat, LfVec3f translation) {
    LfMat4 ret = mat;
    mat_set_row(&ret, 4, 
        vec4f_add(
            vec4f_add(
                vec4f_mul_scaler(mat_get_row(mat, 1), translation.values[0]),
                vec4f_mul_scaler(mat_get_row(mat, 2), translation.values[1])
            ),
            vec4f_add(
                vec4f_mul_scaler(mat_get_row(mat, 3), translation.values[2]), 
                mat_get_row(mat, 4))));
    return ret;
}

LfMat4 scale_mat(LfMat4 mat, LfVec3f scale) {
    LfMat4 ret = mat;
    mat_set_row(&ret, 1, vec4f_mul_scaler(mat_get_row(mat, 1), scale.values[0]));
    mat_set_row(&ret, 2, vec4f_mul_scaler(mat_get_row(mat, 2), scale.values[1]));
    mat_set_row(&ret, 3, vec4f_mul_scaler(mat_get_row(mat, 3), scale.values[2]));
    return ret;
}

LfMat4 mat_mul(LfMat4 m1, LfMat4 m2) {
    LfVec4f row_A1 = mat_get_row(m1, 1);
    LfVec4f row_A2 = mat_get_row(m1, 2);
    LfVec4f row_A3 = mat_get_row(m1, 3);
    LfVec4f row_A4 = mat_get_row(m1, 4);

    LfVec4f row_B1 = mat_get_row(m2, 1);
    LfVec4f row_B2 = mat_get_row(m2, 2);
    LfVec4f row_B3 = mat_get_row(m2, 3);
    LfVec4f row_B4 = mat_get_row(m2, 4);

    LfMat4 ret = mat_identity();

    mat_set_row(&ret, 1, vec4f_add(
        vec4f_add(
            vec4f_mul_scaler(row_A1, row_B1.values[0]),
            vec4f_mul_scaler(row_A2, row_B1.values[1])
       ), 
        vec4f_add(
            vec4f_mul_scaler(row_A3, row_B1.values[2]),
            vec4f_mul_scaler(row_A4, row_B1.values[3])
        ) 
    ));
    mat_set_row(&ret, 2, vec4f_add(
        vec4f_add(
            vec4f_mul_scaler(row_A1, row_B2.values[0]),
            vec4f_mul_scaler(row_A2, row_B2.values[1])
        ), 
        vec4f_add(
            vec4f_mul_scaler(row_A3, row_B2.values[2]),
            vec4f_mul_scaler(row_A4, row_B2.values[3])
        ) 
    ));
    mat_set_row(&ret, 3, vec4f_add(
        vec4f_add(
            vec4f_mul_scaler(row_A1, row_B3.values[0]),
            vec4f_mul_scaler(row_A2, row_B3.values[1])
        ), 
        vec4f_add(
            vec4f_mul_scaler(row_A3, row_B3.values[2]),
            vec4f_mul_scaler(row_A4, row_B3.values[3])
        ) 
    ));
    mat_set_row(&ret, 4, mat_get_row(m2, 4));
    return ret;
}


LfVec4f vec4_mul_mat(LfVec4f vec, LfMat4 mat) {
    LfVec4f mov_0 = vec4f_create(vec.values[0], vec.values[0], vec.values[0], vec.values[0]);
    LfVec4f mov_1 = vec4f_create(vec.values[1], vec.values[1], vec.values[1], vec.values[1]);
    LfVec4f mul_0 = vec4f_mul(mat_get_row(mat, 1), mov_0);
    LfVec4f mul_1 = vec4f_mul(mat_get_row(mat, 2), mov_1);
    LfVec4f add_0 = vec4f_add(mul_0, mul_1);

    LfVec4f mov_2 = vec4f_create(vec.values[2], vec.values[2], vec.values[2], vec.values[2]);
    LfVec4f mov_3 = vec4f_create(vec.values[3], vec.values[3], vec.values[3], vec.values[3]);
    LfVec4f mul_2 = vec4f_mul(mat_get_row(mat, 3), mov_2);
    LfVec4f mul_3 = vec4f_mul(mat_get_row(mat, 4), mov_3);
    LfVec4f add_1 = vec4f_add(mul_2, mul_3);
    LfVec4f add_2 = vec4f_add(add_0, add_1);
    return add_2;
}

LfMat4 orth_mat(float left, float right, float bottom, float top) {
    LfMat4 mat = mat_identity();
    mat_set_row_val(&mat, 1, 1, 2 / (right - left));
    mat_set_row_val(&mat, 2, 2, 2 / (top - bottom));
    mat_set_row_val(&mat, 3, 3, -1);
    mat_set_row_val(&mat, 4, 1, -(right + left) / (right - left));
    mat_set_row_val(&mat, 4, 2, -(top + bottom) / (top - bottom));
    return mat;
}

void mat_set_row_val(LfMat4* mat, uint32_t row_index, uint32_t vec_index, float val) {
    if((row_index > 4 || row_index < 1) || (vec_index > 4 || vec_index < 1)) return;
    row_index--;
    vec_index--;
    mat->values[row_index * 4 + vec_index] = val;
}

float mat_get_row_val(LfMat4 mat, uint32_t row_index, uint32_t vec_index) {
    if((row_index > 4 || row_index < 1) || (vec_index > 4 || vec_index < 1)) return 0;
    return mat.values[row_index * 4 + vec_index];
}

void mat_set_row(LfMat4* mat, uint32_t row_index, LfVec4f val) {
    if(row_index > 4 || row_index < 1) return;
    row_index--;
    mat->values[row_index * 4 + 0] = val.values[0];
    mat->values[row_index * 4 + 1] = val.values[1];
    mat->values[row_index * 4 + 2] = val.values[2];
    mat->values[row_index * 4 + 3] = val.values[3];
}
LfVec4f mat_get_row(LfMat4 mat, uint32_t row_index) {
    if(row_index > 4 || row_index < 1) return (LfVec4f){0};
    row_index--;
    LfVec4f val;
    val.values[0] = mat.values[row_index * 4 + 0];
    val.values[1] = mat.values[row_index * 4 + 1];
    val.values[2] = mat.values[row_index * 4 + 2];
    val.values[3] = mat.values[row_index * 4 + 3];
    return val;
    LfMat4 proj = orth_mat(0.0f, (float)state.dsp_w, (float)state.dsp_h, 0.0f);
}
LfVec4f vec4f_create(float x, float y, float z, float w) {
    LfVec4f vec;
    vec.values[0] = x;
    vec.values[1] = y;
    vec.values[2] = z;
    vec.values[3] = w;
    return vec;
}
LfVec3f vec3f_create(float x, float y, float z) {
    LfVec3f vec;
    vec.values[0] = x;
    vec.values[1] = y;
    vec.values[2] = z;
    return vec;
}
LfVec2f vec2f_create(float x, float y) {
    LfVec2f vec;
    vec.values[0] = x;
    vec.values[1] = y;
    return vec;
}

LfVec4f vec4f_add(LfVec4f a, LfVec4f b) {
    LfVec4f vec;
    vec.values[0] = a.values[0] + b.values[0];
    vec.values[1] = a.values[1] + b.values[1];
    vec.values[2] = a.values[2] + b.values[2];
    vec.values[3] = a.values[3] + b.values[3];
    return vec;
}

LfVec4f vec4f_mul_scaler(LfVec4f v, float k) {
    LfVec4f vec;
    vec.values[0] = v.values[0] * k;
    vec.values[1] = v.values[1] * k;
    vec.values[2] = v.values[2] * k;
    vec.values[3] = v.values[3] * k;
    return vec;
}

LfVec4f vec4f_mul(LfVec4f v1, LfVec4f v2) {
    return vec4f_create(v1.values[0] * v2.values[0], 
                        v1.values[1] * v2.values[1],
                        v1.values[2] * v2.values[2], 
                        v1.values[3] * v2.values[3]);
}

bool point_intersects_aabb(LfVec2f p, LfAABB aabb) {
    return p.values[0] <= (aabb.size.values[0] + (aabb.size.values[0] / 2.0f)) && p.values[0] >= (aabb.pos.values[0] - (aabb.size.values[0] / 2.0f)) && 
        p.values[1] <= (aabb.pos.values[1] + (aabb.size.values[1] / 2.0f)) && p.values[1] >= (aabb.pos.values[1] - (aabb.size.values[1] / 2.0f));
}

bool aabb_intersects_aabb(LfAABB a, LfAABB b) {
    if(a.pos.values[0] + a.size.values[0] / 2 < b.pos.values[0] - b.size.values[0] 
        || b.pos.values[0] + b.pos.values[0] < a.pos.values[0] - a.size.values[0]) return false;

    if(a.pos.values[1] + a.size.values[1] / 2 < b.pos.values[1] - b.size.values[1] 
        || b.pos.values[1] + b.pos.values[1] < a.pos.values[1] - a.size.values[1]) return false;
    return true;
}
LfClickableItemState clickable_item(LfVec2i pos, LfVec2i size, LfUIElementProps props) {
    if(!state.current_div.init) {
        LF_ERROR("Trying to render clickable item without div context. Call lf_div_begin()");
        return(LfClickableItemState){0};
    }
    bool hovered = lf_get_mouse_x() <= (pos.values[0] + size.values[0]) && lf_get_mouse_x() >= (pos.values[0]) && 
        lf_get_mouse_y() <= (pos.values[1] + size.values[1]) && lf_get_mouse_y() >= (pos.values[1]);

    if(hovered && lf_mouse_button_went_down(GLFW_MOUSE_BUTTON_LEFT)) {
        lf_rect(pos, size, props.clicked_color);
        return LF_CLICKABLE_ITEM_STATE_CLICKED;
    }
    if(hovered && (!lf_mouse_button_went_down(GLFW_MOUSE_BUTTON_LEFT) && !lf_mouse_button_is_down(GLFW_MOUSE_BUTTON_LEFT))) {
        lf_rect(pos, size, props.hover_color);
        return LF_CLICKABLE_ITEM_STATE_HOVERED;
    }
    lf_rect(pos, size, props.color);
    return LF_CLICKABLE_ITEM_STATE_IDLE;
}

#ifdef LF_GLFW
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)mods;
    (void)scancode;
    if(action != GLFW_RELEASE) {
        if(!state.input.keyboard.keys[key]) 
            state.input.keyboard.keys[key] = true;
    }  else {
        state.input.keyboard.keys[key] = false;
    }
    state.input.keyboard.keys_changed[key] = (action != GLFW_REPEAT);
    for(uint32_t i = 0; i < state.input.key_cb_count; i++) {
        state.input.key_cbs[i](window, key, scancode, action, mods);
    } 
}
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    if(action != GLFW_RELEASE)  {
        if(!state.input.mouse.buttons_current[button])
            state.input.mouse.buttons_current[button] = true;
    } else {
        state.input.mouse.buttons_current[button] = false;
    }
    for(uint32_t i = 0; i < state.input.mouse_button_cb_count; i++) {
        state.input.mouse_button_cbs[i](window, button, action, mods);
    }
}
void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    state.input.mouse.xscroll_delta = xoffset;
    state.input.mouse.yscroll_delta = yoffset;
    for(uint32_t i = 0; i< state.input.scroll_cb_count; i++) {
        state.input.scroll_cbs[i](window, xoffset, yoffset);
    }
}
void glfw_cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    state.input.mouse.xpos = xpos;
    state.input.mouse.ypos = ypos;

    if(state.input.mouse.first_mouse_press) {
        state.input.mouse.xpos_last = xpos;
        state.input.mouse.ypos_last = ypos;
        state.input.mouse.first_mouse_press = false;
    }
    state.input.mouse.xpos_delta = state.input.mouse.xpos - state.input.mouse.xpos_last;
    state.input.mouse.ypos_delta = state.input.mouse.ypos - state.input.mouse.ypos_last;
    state.input.mouse.xpos_last = xpos;
    state.input.mouse.ypos_last = ypos;
    for(uint32_t i = 0; i< state.input.cursor_pos_cb_count; i++) {
        state.input.cursor_pos_cbs[i](window, xpos, ypos);
    }
}
#endif

// --- Public API Functions ---
//
void lf_init_glfw(uint32_t display_width, uint32_t display_height, LfTheme* theme, void* glfw_window) {
#ifndef LF_GLFW
    LF_ERROR("Trying to initialize Leif with GLFW without defining 'LF_GLFW'");
    return;
#else 
    if(!glfwInit()) {
        LF_ERROR("Trying to initialize Leif with GLFW without initializing GLFW first.");
        return;
    }
    memset(&state, 0, sizeof(state));
    state.init = true;
    state.dsp_w = display_width;
    state.dsp_h = display_height;
    state.window_handle = glfw_window;
    state.input.mouse.first_mouse_press = true;
    state.render.tex_count = 0;
    state.pos_ptr = (LfVec2i){0, 0};
    if(theme != NULL) {
        if(!state.theme.font.cdata) {
            LF_ERROR("No valid font specified for theme.\n");
        }
        state.theme = *theme;
    } else {
        state.theme.div_props = (LfUIElementProps){
            .color = (LfVec4f){LF_RGBA(45, 45, 45, 255)}, 
            .hover_color = (LfVec4f){LF_RGBA(45, 45, 45, 255)},
            .clicked_color = (LfVec4f){LF_RGBA(45, 45, 45, 255)},
        };
        state.theme.text_props = (LfUIElementProps){
            .text_color = (LfVec4f){LF_RGBA(255, 255, 255, 255)}, 
            .margin_left = 10, 
            .margin_right = 10, 
            .margin_top = 10, 
            .margin_bottom = 10
        };
        state.theme.button_props = (LfUIElementProps){ 
            .color = (LfVec4f){LF_RGBA(25, 25, 25, 255)}, 
            .clicked_color = (LfVec4f){LF_RGBA(55, 55, 55, 255)}, 
            .hover_color = (LfVec4f){LF_RGBA(0, 0, 0, 255)}, 
            .text_color = (LfVec4f){LF_RGBA(255, 255, 255, 255)}, 
            .padding = 10,
            .margin_left = 10, 
            .margin_right = 10, 
            .margin_top = 10, 
            .margin_bottom = 10
        };
        state.theme.image_props = (LfUIElementProps){ 
            .color = (LfVec4f){LF_RGBA(255, 255, 255, 255)}, 
            .margin_left = 10, 
            .margin_right = 10, 
            .margin_top = 10, 
            .margin_bottom = 10
        };

        state.theme.font = lf_load_font("/home/cococry/dev/leif/test/fonts/product_sans_regular.ttf", 24.0f, 1024, 1024, 256, 5); 
    }
    glfwSetKeyCallback(state.window_handle, glfw_key_callback);
    glfwSetMouseButtonCallback(state.window_handle, glfw_mouse_button_callback);
    glfwSetScrollCallback(state.window_handle, glfw_scroll_callback);
    glfwSetCursorPosCallback(state.window_handle, glfw_cursor_callback);

    renderer_init();
#endif
}
// --- Public API Functions ---
void lf_init(uint32_t display_width, uint32_t display_height) {
    state.init = true;
    state.dsp_w = display_width;
    state.dsp_h = display_height;

    renderer_init();
}

void lf_resize_display(uint32_t display_width, uint32_t display_height) {
    state.dsp_w = display_width;
    state.dsp_h = display_height;
    LfMat4 proj = orth_mat(0.0f, (float)state.dsp_w, (float)state.dsp_h, 0.0f);
    state.current_div.aabb.size.values[0] = state.dsp_w;
    state.current_div.aabb.size.values[1] = state.dsp_h;
    shader_set_mat(state.render.shader, "u_proj", proj);
}


void lf_rect(LfVec2i pos, LfVec2i size, LfVec4f color) {
    pos = (LfVec2i){pos.values[0] + size.values[0] / 2.0f, pos.values[1] + size.values[1] / 2.0f};

    LfVec2f texcoords[6] = {
        (LfVec2f){0.0f, 0.0f},
        (LfVec2f){0.0f, 1.0f},
        (LfVec2f){1.0f, 1.0f},

        (LfVec2f){0.0f, 0.0f},
        (LfVec2f){1.0f, 1.0f},
        (LfVec2f){1.0f, 0.0f},
    };
    LfMat4 transform = mat_mul(
        scale_mat(mat_identity(), vec3f_create(size.values[0], size.values[1], 0.0f)),
        translate_mat(mat_identity(), vec3f_create((float)pos.values[0], (float)pos.values[1], 0.0f))
    );
    for(uint32_t i = 0; i < 6; i++) {
        LfVec4f pos_mat = vec4_mul_mat(state.render.vert_pos[i], transform);
        state.render.verts[state.render.vert_count].pos = vec2f_create(pos_mat.values[0], 
            pos_mat.values[1]);
        state.render.verts[state.render.vert_count].color = color;
        state.render.verts[state.render.vert_count].texcoord = texcoords[i];
        state.render.verts[state.render.vert_count].tex_index = -1.0f;
        state.render.vert_count++;
    } 
}

void lf_image_render(LfVec2i pos, LfVec4f color, LfTexture tex) {
    pos = (LfVec2i){pos.values[0] + tex.width / 2.0f, pos.values[1] + tex.height / 2.0f};
    LfVec2f texcoords[6] = {
        (LfVec2f){0.0f, 0.0f},
        (LfVec2f){0.0f, 1.0f},
        (LfVec2f){1.0f, 1.0f},

        (LfVec2f){0.0f, 0.0f},
        (LfVec2f){1.0f, 1.0f},
        (LfVec2f){1.0f, 0.0f},
    };
    float tex_index = -1.0f;
    for(uint32_t i = 0; i < state.render.tex_count; i++) {
        if(tex.id == state.render.textures[i].id)  {
            tex_index = i;
            break;
        }
    }
    if(tex_index == -1.0f) {
        tex_index = (float)state.render.tex_index;
        state.render.textures[state.render.tex_count++] = tex;
        state.render.tex_index++;
    }
    LfMat4 transform = mat_mul(
        scale_mat(mat_identity(), vec3f_create(tex.width, tex.height, 0.0f)),
        translate_mat(mat_identity(), vec3f_create((float)pos.values[0], (float)pos.values[1], 0.0f))
    );
    for(uint32_t i = 0; i < 6; i++) {
        LfVec4f pos_mat = vec4_mul_mat(state.render.vert_pos[i], transform);
        state.render.verts[state.render.vert_count].pos = vec2f_create(pos_mat.values[0], 
            pos_mat.values[1]);
        state.render.verts[state.render.vert_count].color = color;
        state.render.verts[state.render.vert_count].texcoord = texcoords[i];
        state.render.verts[state.render.vert_count].tex_index = tex_index;
        state.render.vert_count++;
    } 
}
LfTextProps lf_text_render(LfVec2i pos, const char* str, LfFont font, LfUIElementProps props, float wrap_point, bool no_wrap, bool no_render) {
    float tex_index = -1.0f;
    if(!no_render) {
        for(uint32_t i = 0; i < state.render.tex_count; i++) {
            if(state.render.textures[i].id == font.bitmap.id) {
                tex_index = (float)i;
                break;
            }
        }
        if(tex_index == -1.0f) {
            tex_index = (float)state.render.tex_index;
            LfTexture tex = font.bitmap;
            state.render.textures[state.render.tex_count++] = tex;
            state.render.tex_index++;
        }
    }
    float x = pos.values[0];
    float y = pos.values[1];
    int32_t max_char_height = -1;
    int32_t max_y_vert = -1;
    float width = 0;
    bool new_line = false;
    for(uint32_t i = 0; i < strlen(str); i++) {
        float scale = stbtt_ScaleForPixelHeight(font.font_info, font.font_size);

        int width, height;
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(font.font_info, str[i], scale, scale, &x0, &y0, &x1, &y1);
        height = y1 - y0;
        if(height > max_char_height) {
            max_char_height = height;
        }
        if(y1 > max_y_vert) {
            max_y_vert = y1;
        }
    }
    while(*str) { 
        bool skip = false;
        if(*str == '\n' || (x >= wrap_point && !no_wrap)) {
            if(x > width)
                x = width;
            y += (font.font_size + font.line_gap_add) / 1.5f;
            x = pos.values[0];
            if(*str == '\n' || *str == ' ') {
                skip = true; 
            }
            if(*str == '\n')
                new_line = true;
        }
        if(!skip) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad((stbtt_bakedchar*)font.cdata, font.tex_width, font.tex_height, *str-32, &x, &y, &q, 1);
            if(!no_render)  { 
                for(uint32_t i = 0; i < 6; i++) {
                    LfVec2f verts[6] = {
                        (LfVec2f){q.x0, q.y0}, 
                        (LfVec2f){q.x0, q.y1}, 
                        (LfVec2f){q.x1, q.y1},

                        (LfVec2f){q.x0, q.y0},
                        (LfVec2f){q.x1, q.y1}, 
                        (LfVec2f){q.x1, q.y0}
                    };
                    for(uint32_t i = 0; i < 6; i++) {
                        verts[i].values[1] += max_char_height - (max_y_vert); 
                    }
                    LfVec2f texcoords[6] = {
                        q.s0, q.t0, 
                        q.s0, q.t1, 
                        q.s1, q.t1, 

                        q.s0, q.t0, 
                        q.s1, q.t1, 
                        q.s1, q.t0
                    };
                    state.render.verts[state.render.vert_count].pos = verts[i]; 
                    state.render.verts[state.render.vert_count].color = props.text_color;
                    state.render.verts[state.render.vert_count].texcoord = texcoords[i];
                    state.render.verts[state.render.vert_count].tex_index = tex_index;
                    state.render.vert_count++;
                }
            }
        }
        str++;
    }
    if(!new_line)
        width = x -pos.values[0];
    return (LfTextProps){.width = width, .height = max_char_height };
}

void lf_flush() {
    if(state.render.vert_count <= 0) return;

    glUseProgram(state.render.shader.id);
    glBindBuffer(GL_ARRAY_BUFFER, state.render.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * state.render.vert_count, 
                    state.render.verts);

    for(uint32_t i = 0; i < MAX_TEX_COUNT_BATCH; i++) {
        glBindTextureUnit(i, state.render.textures[i].id);
    }

    glBindVertexArray(state.render.vao);
    glDrawArrays(GL_TRIANGLES, 0, state.render.vert_count);

    state.render.vert_count = 0;
    state.render.tex_index = 0;
}

void lf_add_key_callback(void* cb) {
    state.input.key_cbs[state.input.key_cb_count++] = cb;
}
void lf_add_mouse_button_callback(void* cb) {
    state.input.mouse_button_cbs[state.input.mouse_button_cb_count++] = cb;
}

void lf_add_scroll_callback(void* cb) {
    state.input.scroll_cbs[state.input.scroll_cb_count++] = cb;
}

void lf_add_cursor_pos_callback(void* cb) {
    state.input.cursor_pos_cbs[state.input.cursor_pos_cb_count++] = cb;
}

bool lf_key_went_down(uint32_t key) {
    return lf_key_changed(key) && state.input.keyboard.keys[key];
}

bool lf_key_is_down(uint32_t key) {
    return state.input.keyboard.keys[key];
}

bool lf_key_is_released(uint32_t key) {
    return lf_key_changed(key) && !state.input.keyboard.keys[key];
}

bool lf_key_changed(uint32_t key) {
    bool ret = state.input.keyboard.keys_changed[key];
    state.input.keyboard.keys_changed[key] = false;
    return ret;
}

bool lf_mouse_button_went_down(uint32_t button) {
    return lf_mouse_button_changed(button) && state.input.mouse.buttons_current[button];
}

bool lf_mouse_button_is_down(uint32_t button) {
    return state.input.mouse.buttons_current[button];
}

bool lf_mouse_button_is_released(uint32_t button) {
    return lf_mouse_button_changed(button) && !state.input.mouse.buttons_current[button];
}

bool lf_mouse_button_changed(uint32_t button) {
    return state.input.mouse.buttons_current[button] != state.input.mouse.buttons_last[button];
}

double lf_get_mouse_x() {
    return state.input.mouse.xpos;
}

double lf_get_mouse_y() {
    return state.input.mouse.ypos;
}

double lf_get_mouse_x_delta() {
    return state.input.mouse.xpos_delta;
}

double lf_get_mouse_y_delta() {
    return state.input.mouse.ypos_delta;
}

double lf_get_mouse_scroll_x() {
    return state.input.mouse.xscroll_delta;
}

double lf_get_mouse_scroll_y() {
    return state.input.mouse.yscroll_delta;
}

LfClickableItemState lf_button(const char* text) {
    float padding = state.theme.button_props.padding;
    float margin_left = state.theme.button_props.margin_left, margin_right = state.theme.button_props.margin_right, 
        margin_top = state.theme.button_props.margin_top, margin_bottom = state.theme.button_props.margin_bottom;
    LfTextProps text_props = lf_text_render(state.pos_ptr, text, state.theme.font, state.theme.button_props, state.current_div.aabb.size.values[0] - margin_right * 2.0f - padding * 2.0f, true, true);
    if(text_props.height + padding * 2 + margin_bottom  > state.current_line_height) {
        state.current_line_height = text_props.height + padding * 2 + margin_bottom;
    }
    if((state.pos_ptr.values[0] - state.current_div.aabb.pos.values[0] + text_props.width + padding * 2.0f) + margin_right + margin_left >= state.current_div.aabb.size.values[0]) {
        lf_next_line();
    }
    state.pos_ptr.values[0] += margin_left;
    state.pos_ptr.values[1] += margin_top;
    if(state.pos_ptr.values[1] + text_props.height + margin_bottom + padding * 2.0f >= state.current_div.aabb.size.values[1] + state.current_div.aabb.pos.values[1]) return LF_CLICKABLE_ITEM_STATE_IDLE;
    LfClickableItemState ret = clickable_item(state.pos_ptr, (LfVec2i){text_props.width + padding * 2, text_props.height + padding * 2}, state.theme.button_props);
    lf_text_render((LfVec2i){state.pos_ptr.values[0] + padding, state.pos_ptr.values[1] + padding}, text, state.theme.font, state.theme.button_props, state.current_div.aabb.size.values[0] - margin_right * 2.0f - padding * 2.0f,
                   true, false);

    state.pos_ptr.values[0] += text_props.width + margin_right+ padding;
    state.pos_ptr.values[1] -= margin_top;
    return ret;
}
LfClickableItemState lf_button_fixed(const char* text, int32_t width, int32_t height) {
    float padding = state.theme.button_props.padding;
    float margin_left = state.theme.button_props.margin_left, margin_right = state.theme.button_props.margin_right, 
        margin_top = state.theme.button_props.margin_top, margin_bottom = state.theme.button_props.margin_bottom;
    LfTextProps text_props = lf_text_render(state.pos_ptr, text, state.theme.font, state.theme.button_props, state.current_div.aabb.size.values[0] - margin_right * 2.0f - padding * 2.0f, true, true);
    if(text_props.height + padding * 2 + margin_bottom  > state.current_line_height) {
        state.current_line_height = text_props.height + padding * 2 + margin_bottom;
    }
    if((state.pos_ptr.values[0] - state.current_div.aabb.pos.values[0] + text_props.width + padding * 2.0f) + margin_right + margin_left >= state.current_div.aabb.size.values[0]) {
        lf_next_line();
    }
    state.pos_ptr.values[0] += margin_left;
    state.pos_ptr.values[1] += margin_top;
    if(state.pos_ptr.values[1] + text_props.height + margin_bottom + padding * 2.0f >= state.current_div.aabb.size.values[1] + state.current_div.aabb.pos.values[1]) return LF_CLICKABLE_ITEM_STATE_IDLE;
    LfClickableItemState ret = clickable_item(state.pos_ptr, (LfVec2i){width == -1 ? text_props.width : width + padding * 2, ((height == -1) ? text_props.height : height) + padding * 2}, state.theme.button_props);
    lf_text_render((LfVec2i){state.pos_ptr.values[0] + padding + ((width != -1) ? width / 2.0f - text_props.width / 2.0f : 0), state.pos_ptr.values[1] + padding}, text, state.theme.font, state.theme.button_props, state.current_div.aabb.size.values[0] - margin_right * 2.0f - padding * 2.0f,
                   true, false);

    state.pos_ptr.values[0] += ((width == -1) ? text_props.width : width) + margin_right + padding;
    state.pos_ptr.values[1] -= margin_top;
    return ret;
}

LfClickableItemState lf_div_begin(LfVec2i pos, LfVec2i size) {
    if(!state.current_div.init_size) {
        state.current_div.aabb.pos = pos;
        state.current_div.aabb.size = size;
        state.current_div.init_size = true;
    }
    state.current_div.init = true;
    state.current_div.interact_state = clickable_item(state.current_div.aabb.pos, state.current_div.aabb.size, state.theme.div_props);
    state.pos_ptr = pos;
    return state.current_div.interact_state;
}

void lf_div_end() {
    state.current_div.init = false;
}

LfUIElementProps lf_syle_color(LfVec4f color) {
    return (LfUIElementProps){
        .hover_color = color, 
        .color = color, 
        .clicked_color = color
    };
}

void lf_next_line() {
    state.pos_ptr.values[0] = state.current_div.aabb.pos.values[0];
    state.pos_ptr.values[1] += state.current_line_height;
    state.current_line_height = 0;
}

void lf_update_input() {
    memcpy(state.input.mouse.buttons_last, state.input.mouse.buttons_current, sizeof(bool) * MAX_MOUSE_BUTTONS);
}
LfTextProps lf_get_text_props(const char* str) {
    return lf_text_render(state.pos_ptr, str, state.theme.font, state.theme.text_props, 
                          state.current_div.aabb.pos.values[0] + state.current_div.aabb.size.values[0], false,  true);
}


void lf_text(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    float padding = state.theme.text_props.padding;
    float margin_left = state.theme.text_props.margin_left, margin_right = state.theme.text_props.margin_right, 
        margin_top = state.theme.text_props.margin_top, margin_bottom = state.theme.text_props.margin_bottom;

    LfTextProps text_props = lf_text_render(state.pos_ptr, buf, state.theme.font, state.theme.text_props, state.current_div.aabb.size.values[0] - margin_right * 2.0f - padding * 2.0f, false, true);
    if(text_props.height + padding * 2 + margin_bottom  > state.current_line_height) {
        state.current_line_height = text_props.height + padding * 2 + margin_bottom;
    }
    if((state.pos_ptr.values[0] - state.current_div.aabb.pos.values[0] + text_props.width + padding * 2.0f) + margin_right + margin_left >= state.current_div.aabb.size.values[0]) {
        lf_next_line();
    }
    state.pos_ptr.values[0] += margin_left;
    state.pos_ptr.values[1] += margin_top;
    if(state.pos_ptr.values[1] + text_props.height + margin_bottom + padding * 2.0f >= state.current_div.aabb.size.values[1] + state.current_div.aabb.pos.values[1]) return;

    lf_rect(state.pos_ptr, (LfVec2i){text_props.width + padding * 2.0f, text_props.height + padding * 2.0f}, state.theme.text_props.color);
    lf_text_render((LfVec2i){state.pos_ptr.values[0] + padding, state.pos_ptr.values[1] + padding}, buf, state.theme.font, state.theme.text_props, state.current_div.aabb.pos.values[0] + state.current_div.aabb.size.values[0] - margin_left - margin_right - padding * 2.0f, false, false);
    state.pos_ptr.values[0] += text_props.width + margin_right+ padding;
    state.pos_ptr.values[1] -= margin_top;
}


LfVec2i lf_get_div_size() {
    return state.current_div.aabb.size;
}

void lf_set_ptr_x(float x) {
    state.pos_ptr.values[0] = x + state.current_div.aabb.pos.values[0];
}

void lf_set_ptr_y(float y) {
    state.pos_ptr.values[1] = y + state.current_div.aabb.pos.values[1];
}


void lf_image(uint32_t tex_id, uint32_t width, uint32_t height) {
    float margin_left = state.theme.image_props.margin_left, margin_right = state.theme.image_props.margin_right, 
        margin_top = state.theme.image_props.margin_top, margin_bottom = state.theme.image_props.margin_bottom;
    
    if((state.pos_ptr.values[0] - state.current_div.aabb.pos.values[0] + width) + margin_right + margin_left >= state.current_div.aabb.size.values[0]) {
        lf_next_line();
    }
    if(state.pos_ptr.values[1] + height + margin_bottom >= state.current_div.aabb.size.values[1] + state.current_div.aabb.pos.values[1]) return;   
    state.pos_ptr.values[0] += margin_left; 
    state.pos_ptr.values[1] += margin_top;
    lf_image_render(state.pos_ptr, state.theme.image_props.color, (LfTexture){.id = tex_id, .width = width, .height = height});
    state.pos_ptr.values[0] += width + margin_right;
    state.pos_ptr.values[1] -= margin_top;
    if((int32_t)height >= state.current_line_height)
        state.current_line_height = height;
}
LfTheme* lf_theme() {
    return &state.theme;
}
