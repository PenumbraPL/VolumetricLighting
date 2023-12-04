// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "VolumetricLighting.h"

WindowInfo windowConfig = {
    1900,
    1000,
    "GLTF Viewer",
    0, 0, 0
};
ConfigContext panel_config{
    2000.f, 0.f, 50, 0, 0, 0, 0, 0, 50, 0, 0
};
double xpos, ypos;
float mouse_speed = 2.f;
std::vector<glm::mat4x4*> mats;
glm::vec4 cam;
std::map <void*, unsigned int> bufferViews;
std::map <void*, unsigned int> textureViews;
std::vector<Primitive> primitives;

void print_map(const std::map<void*, unsigned int>& m){
      for (const auto& n : m)
          std::cout << n.first << " = " << n.second << "; ";
    std::cout << '\n';
}


void formatAttribute(GLint attr_location, AkAccessor* acc) {
    int comp_size;
    int type;
    GLuint normalize;
    size_t offset;
    int comp_stride;
    size_t length;

    comp_size = acc->componentSize;
    type = acc->componentType;
    normalize = acc->normalized ? GL_TRUE : GL_FALSE;
    offset = acc->byteOffset;
    comp_stride = acc->componentBytes;
    length = acc->byteLength;


    switch (comp_size) {
    case AK_COMPONENT_SIZE_SCALAR:                comp_size = 1; break;
    case AK_COMPONENT_SIZE_VEC2:                  comp_size = 2; break;
    case AK_COMPONENT_SIZE_VEC3:                  comp_size = 3; break;
    case AK_COMPONENT_SIZE_VEC4:                  comp_size = 4; break;
    case AK_COMPONENT_SIZE_MAT2:                  comp_size = 4; break;
    case AK_COMPONENT_SIZE_MAT3:                  comp_size = 9; break;
    case AK_COMPONENT_SIZE_MAT4:                  comp_size = 16; break;
    case AK_COMPONENT_SIZE_UNKNOWN:
    default:                                      comp_size = 1; break;
    }

    switch (type) {
    case AKT_FLOAT:						type = GL_FLOAT; break;
    case AKT_UINT:						type = GL_UNSIGNED_INT; break;
    case AKT_BYTE:						type = GL_BYTE; break;
    case AKT_UBYTE:						type = GL_UNSIGNED_BYTE; break;
    case AKT_SHORT:						type = GL_SHORT; break;
    case AKT_USHORT:					type = GL_UNSIGNED_SHORT; break;
    case AKT_UNKNOWN:
    case AKT_NONE:
    default:                            type = GL_INT; break;
    };

    std::cout << length << " " << comp_size << " " << type << " " << offset << " " << comp_stride << std::endl;

    glVertexAttribFormat(attr_location, comp_size, type, normalize, 0);
}


void*
imageLoadFromFile(const char* __restrict path,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) {
    return stbi_load(path, width, height, components, 0);
}

void*
imageLoadFromMemory(const char* __restrict data,
    size_t                  len,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) {
    return stbi_load_from_memory((stbi_uc const*)data, (int)len, width, height, components, 0);
}

void
imageFlipVerticallyOnLoad(bool flip) {
    stbi_set_flip_vertically_on_load(flip);
}


GLuint wrap_mode(AkWrapMode& wrap) {
    GLuint wrap_m = GL_REPEAT;
    switch (wrap) {
    case AK_WRAP_MODE_WRAP: wrap_m = GL_REPEAT; break;
    case AK_WRAP_MODE_MIRROR: wrap_m = GL_MIRRORED_REPEAT; break;
    case AK_WRAP_MODE_CLAMP: wrap_m = GL_CLAMP_TO_EDGE; break;
    case AK_WRAP_MODE_BORDER: wrap_m = GL_CLAMP_TO_BORDER; break;
    case AK_WRAP_MODE_MIRROR_ONCE: wrap_m = GL_MIRROR_CLAMP_TO_EDGE; break;
    }
    return wrap_m;
}

void set_up_color(AkColorDesc* colordesc, AkMeshPrimitive* prim, GLuint** sampler, GLuint** texture) {
    if (colordesc) {
        if (colordesc->texture) {
            AkTextureRef* tex = colordesc->texture;
            if (tex->texture) {
                AkSampler* samp = tex->texture->sampler;
                if (!samp) return;

                AkTypeId type = tex->texture->type;
                //std::cout << "Texture path: " << tex->texture->image->initFrom->ref;


                GLuint texture_type;
                GLuint minfilter, magfilter, mipfilter;
                GLuint wrap_t, wrap_s, wrap_p;
                switch (type) {
                case AKT_SAMPLER1D:     texture_type = GL_TEXTURE_1D; break;
                case AKT_SAMPLER2D:     texture_type = GL_TEXTURE_2D; break;
                case AKT_SAMPLER3D:     texture_type = GL_TEXTURE_3D; break;
                case AKT_SAMPLER_CUBE:  texture_type = GL_TEXTURE_CUBE_MAP; break;
                case AKT_SAMPLER_RECT:  texture_type = GL_TEXTURE_RECTANGLE; break;
                case AKT_SAMPLER_DEPTH: texture_type = GL_TEXTURE_2D; break;
                    break;
                }
                wrap_t = wrap_mode(samp->wrapT);
                wrap_s = wrap_mode(samp->wrapS);
                wrap_p = wrap_mode(samp->wrapP);

                //GLuint border_color[] = {samp->borderColor->rgba.R, samp->borderColor->rgba.G,
                //    samp->borderColor->rgba.B, samp->borderColor->rgba.A};
                switch (samp->minfilter) {
                case AK_MINFILTER_LINEAR:       minfilter = GL_LINEAR; break;
                case AK_MINFILTER_NEAREST:      minfilter = GL_NEAREST; break;
                case AK_LINEAR_MIPMAP_NEAREST:  minfilter = GL_LINEAR_MIPMAP_NEAREST; break;
                case AK_LINEAR_MIPMAP_LINEAR:   minfilter = GL_LINEAR_MIPMAP_LINEAR; break;
                case AK_NEAREST_MIPMAP_NEAREST: minfilter = GL_NEAREST_MIPMAP_NEAREST; break;
                case AK_NEAREST_MIPMAP_LINEAR:  minfilter = GL_NEAREST_MIPMAP_LINEAR; break;
                }

                switch (samp->magfilter) {
                case AK_MAGFILTER_LINEAR:   magfilter = GL_LINEAR; break;
                case AK_MAGFILTER_NEAREST:  magfilter = GL_NEAREST; break;
                }

                switch (samp->mipfilter) {
                case AK_MIPFILTER_LINEAR:   mipfilter = GL_LINEAR; break;
                case AK_MIPFILTER_NEAREST:  mipfilter = GL_NEAREST; break;
                case AK_MIPFILTER_NONE:  mipfilter = GL_NONE; break;
                }
                //GLuint sampler = primitive.sampler;
                *sampler = (GLuint*)calloc(1, sizeof(GLuint));
                glCreateSamplers(1, *sampler);
                glSamplerParameteri(*sampler[0], GL_TEXTURE_WRAP_S, wrap_s);
                glSamplerParameteri(*sampler[0], GL_TEXTURE_WRAP_T, wrap_t);
                glSamplerParameteri(*sampler[0], GL_TEXTURE_WRAP_R, wrap_p);
                glSamplerParameteri(*sampler[0], GL_TEXTURE_MIN_FILTER, minfilter);
                glSamplerParameteri(*sampler[0], GL_TEXTURE_MAG_FILTER, magfilter);
                //glSamplerParameteri(sampler, GL_TEXTURE_BORDER_COLOR, border_color);
                //glBindSampler(0, sampler); //

                AkInput* tex_coord = ak_meshInputGet(prim, tex->coordInputName, tex->slot); //
                int components = 0;
                int width = 0;
                int height = 0;
                char path[128] = { "./res/ship_in_clouds/" };
                const char* f_path = tex->texture->image->initFrom->ref;
                memcpy_s(path + strlen(path), 128 - strlen(path), f_path, strlen(f_path));
                char* image = (char*)imageLoadFromFile(path, &width, &height, &components);


                //GLuint texture = primitive.texture;
                *texture = (GLuint*)calloc(1, sizeof(GLuint));
                glCreateTextures(texture_type, 1, *texture);
                if (image) {
                    glTextureStorage2D(*texture[0], 1, GL_RGB8, width, height);
                    glTextureSubImage2D(*texture[0], 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
                    stbi_image_free(image);
                    //glActiveTexture(GL_TEXTURE0);
                    //glBindTexture(texture_type, texture);
                }


                //GLint units;
                //glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &units);
                //glBindTextureUnit(0, textures[0]); // (binding = 0)
                //glDeleteSamplers(1, &sampler);
                //glDeleteTextures(1, &texture);
            }

        }
    }
}

char* read_file(const char* file_name) {
    FILE* fs;
    fopen_s(&fs, file_name, "rb");

    if (!fs) {
        return nullptr;
    }

    fseek(fs, 0, SEEK_END);
    int file_size = ftell(fs);
    rewind(fs);

    char* buffer = (char*)calloc(file_size + 1, 1);
    if(buffer) fread(buffer, 1, file_size, fs);
    fclose(fs);

    return buffer;
}

GLint checkPipelineStatus(GLuint vertex_shader, GLuint fragment_shader) {
    GLint v_comp_status, f_comp_status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_comp_status);
    if (!v_comp_status) {
        char comp_info[1024];
        memset(comp_info, '\0', 1024);
        //glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(vertex_shader, 1024, NULL, comp_info);
        std::cout << "Vertex Shader: ";
        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_comp_status);
    if (!f_comp_status) {
        char comp_info[1024];
        memset(comp_info, '\0', 1024);
        //glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(fragment_shader, 1024, NULL, comp_info);
        std::cout << "Fragment Shader: ";
        fwrite(comp_info, 1024, 1, stdout);
    }
    return (!v_comp_status || !f_comp_status) ? 0 : 1;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    panel_config.p6 += yoffset * -6;
}

static void cursor_position_callback(GLFWwindow* window, double new_xpos, double new_ypos)
{
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state != GLFW_RELEASE)
    {
        double nx = (mouse_speed / windowConfig.width) * (new_xpos - xpos);
        double ny = (mouse_speed / windowConfig.height) * (new_ypos - ypos);
        panel_config.p7 += nx;
        panel_config.p8 += ny;
        
        xpos = new_xpos;
        ypos = new_ypos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "Mouse button: " << xpos << ", " << ypos << std::endl;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p7 += 0.01;
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p7 -= 0.01;
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p8 += 0.01;
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p8 -= 0.01;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        GLFWmousebuttonfun mouse_button_callbacks[2] = { NULL, mouse_button_callback };
        windowConfig.mbutton += 1;
        windowConfig.mbutton %= 2;
        glfwSetMouseButtonCallback(window, mouse_button_callbacks[windowConfig.mbutton]);
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        GLFWcursorposfun cursor_callbacks[2] = { NULL, cursor_position_callback };
        windowConfig.imgui += 1;
        windowConfig.imgui %= 2;
        glfwSetCursorPosCallback(window, cursor_callbacks[windowConfig.imgui]);
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        int mode[2] = { GLFW_CURSOR_DISABLED, GLFW_CURSOR_NORMAL };
        windowConfig.cursor_mode += 1;
        windowConfig.cursor_mode %= 2;
        glfwSetInputMode(window, GLFW_CURSOR, mode[windowConfig.cursor_mode % (sizeof(mode)/sizeof(int))]);
    }
}

void setUniformMVP(GLuint Location, glm::vec3 const& Translate, glm::vec3 const& Rotate)
{
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 ViewTranslate = glm::translate(
        glm::mat4(1.0f), Translate);
    glm::mat4 ViewRotateX = glm::rotate(
        ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
    glm::mat4 View = glm::rotate(ViewRotateX,
        Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(
        glm::mat4(1.0f), glm::vec3(0.5f));
    glm::mat4 MVP = Projection * View * Model;
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(MVP));
}

void proccess_node(AkNode* node) {
    uint8_t *raw_buffer;
    unsigned long buffer_size;

    int offset = 0;
    int comp_stride = 0;
    int normalize = 0;
    int type = 0;
    int comp_size = 0;

    Primitive pr;
    std::string geo_type;
    
    float* world_transform = pr.setWorldTransform();
    float* transform = pr.setTransform();
    ak_transformCombineWorld(node, world_transform);
    ak_transformCombine(node, transform);

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node); // if geometry
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        switch ((AkGeometryType)geometry->gdata->type) { //if gdata
        case AK_GEOMETRY_MESH:
            geo_type = "mesh";
            if (mesh) {
                GLuint prim_type;

                for (int i = 0; i < mesh->primitiveCount; i++) {/*prim = prim->next;*/ }
                AkMeshPrimitive* prim = mesh->primitive;
                switch (prim->type) {
                case AK_PRIMITIVE_LINES:              prim_type = GL_LINES; break;
                case AK_PRIMITIVE_POLYGONS:           prim_type = GL_POLYGON; break;
                case AK_PRIMITIVE_TRIANGLES:          prim_type = GL_TRIANGLES; break;
                case AK_PRIMITIVE_POINTS:
                default:                              prim_type = GL_POINTS; break;
                }
                if (prim->indices) {
                    pr.ind = (uint32_t*)prim->indices->items;
                    pr.ind_size = prim->indices->count;
                }
                std::cout << "Mesh name:" << mesh->name << std::endl;   // should i insert mesh->name ??
                std::cout << "Mesh center:" << mesh->center << std::endl; // same
                std::cout << "Primitive center: " << prim->center << std::endl;
                int set = prim->input->set;

                if (prim->material) {
                    AkMaterial* mat = prim->material;
                    AkEffect* ef = (AkEffect*)ak_instanceObject(&mat->effect->base);
                    AkTechniqueFxCommon* tch = ef->profile->technique->common;
                    if (tch) {
                        set_up_color(tch->ambient, prim, &pr.amb_sampler, &pr.amb_texture);
                        set_up_color(tch->emission, prim, &pr.emi_sampler, &pr.emi_texture);
                        set_up_color(tch->diffuse, prim, &pr.diff_sampler, &pr.diff_texture);
                        set_up_color(tch->specular, prim, &pr.spec_sampler, &pr.spec_texture);

                        switch (tch->type) {
                        case AK_MATERIAL_METALLIC_ROUGHNESS:
                            std::cout << "\nmetalic roughness\n";
                            break;
                        case AK_MATERIAL_SPECULAR_GLOSSINES:
                            std::cout << "\nspecular glossines\n";
                            break;
                        };
                        std::cout << "Is double sized: " << (tch->doubleSided ? "True" : "False");
                    }
                }

                AkInput* wgs = ak_meshInputGet(prim, "WEIGHTS", set);
                AkInput* jts = ak_meshInputGet(prim, "JOINTS", set);
                AkInput* pos = ak_meshInputGet(prim, "POSITION", set);
                AkInput* tex = ak_meshInputGet(prim, "TEXCOORD", set); // if indexed then multiple parts to proccess
                AkInput* nor = ak_meshInputGet(prim, "NORMAL", set);

                AkInput* col = ak_meshInputGet(prim, "COLOR", set);
                AkInput* tan = ak_meshInputGet(prim, "TANGENT", set);
                //Color tangent

                //std::cout << ak_meshInputCount(mesh) << std::endl;
                
                pr.wgs = wgs ? wgs->accessor : nullptr;
                pr.jts = jts ? jts->accessor : nullptr;
                pr.pos = pos ? pos->accessor : nullptr;
                pr.tex = tex ? tex->accessor : nullptr;
                pr.nor = nor ? nor->accessor : nullptr;
                pr.col = col ? col->accessor : nullptr;
                pr.tan = tan ? tan->accessor : nullptr;
                
                primitives.push_back(pr);
            };
            break;
        case AK_GEOMETRY_SPLINE: geo_type = "spline"; break;
        case  AK_GEOMETRY_BREP:  geo_type = "brep";   break;
        default:                 geo_type = "other";  break;
        };
    }

    std::cout << "Node name: " << node->name << std::endl;
    std::cout << "Node type: " << geo_type << std::endl;
   
    if (node->next) {
        node = node->next;
        proccess_node(node);
    }
    if (node->chld) {
        node = node->chld;
        proccess_node(node);
    }
}

std::string printCoordSys(AkCoordSys* coord) {
    if (coord) {
        AkAxis axis[] = { coord->axis.fwd,
        coord->axis.right,
        coord->axis.up,
        coord->cameraOrientation.fwd,
        coord->cameraOrientation.right,
        coord->cameraOrientation.up };
        std::string ax_name[] = { "axis FW:" ,"axis RH:" ,"axis UP:", "camera FW:", "camera RH:", "camera UP : "};

        AkAxisRotDirection axis_dir = coord->rotDirection;
        std::string coordString;

        for (int i = 0; i < sizeof(axis)/sizeof(AkAxis); i++) {
            std::string st;
            switch (axis[i]) {
            case AK_AXIS_NEGATIVE_X: st = "NEGATIVE_X"; break;
            case AK_AXIS_NEGATIVE_Y: st = "NEGATIVE_Y"; break;
            case AK_AXIS_NEGATIVE_Z: st = "NEGATIVE_Z"; break;
            case AK_AXIS_POSITIVE_X: st = "POSITIVE_X"; break;
            case AK_AXIS_POSITIVE_Y: st = "POSITIVE_Y"; break;
            case AK_AXIS_POSITIVE_Z: st = "POSITIVE_Z"; break;
            }
            coordString += ax_name[i] + " " + st + "\n";
        }
        switch (axis_dir) {
        case AK_AXIS_ROT_DIR_LH: coordString += "rot dir: ROT LEFT\n";  break;
        case AK_AXIS_ROT_DIR_RH: coordString += "rot dir: ROT RIGHT\n";  break;
        }
        return coordString;
    }
    return "CoordSys is nullptr!\n";
}

std::string printInf(AkDocInf* inf, AkUnit* unit) {
    std::string infString;
    if (inf && unit) {
        infString += "Units: " + std::string(unit->name) + " ";
        infString += unit->dist;
        infString += "\nPath: " + std::string(inf->name);
        infString += "\nFlip Image: ";
        infString += inf->flipImage ? "True" : "False";
        infString+= "\n";
        if (AK_FILE_TYPE_GLTF == inf->ftype) {
            infString += "Type: GLTF\n";
        }else {
            infString += "Unknown type\n";
        }
        
        return infString;
    }
    return "AkDocInf or AkUnit is nullptr!\n";
}




int main(void)
{
    std::cout << "========== Initialization started ============================================\n";
    if (!glfwInit()) {
        std::cout << "========== [GLFW]: Initialization failed =====================================\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "========== [GLFW]: Terminated ================================================\n";
        std::cout << "========== [GLFW]: Window initialization failed ==============================\n";
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(error_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    initializeGLEW();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 450";
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "========== [GLFW]: Debug context initialize successful =======================\n";
        std::vector<DEBUGPROC> callbacks;
        callback_list(callbacks);
        debug_init(callbacks);
    }  else {
        std::cout << "========== [GLFW]: Debug context initialize unsuccessful =====================\n";
    }

    
    char* v_sh_buffer = read_file("res/vertex2.glsl");
    if (!v_sh_buffer){
        std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
    }
    
    char* f_sh_buffer = read_file("res/fragment2.glsl");
    if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";
    

    GLuint vao, vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location, norm_location, tex_location;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glObjectLabel(GL_SHADER, vertex_shader, -1, "Vertex Shader");
    glShaderSource(vertex_shader, 1, &v_sh_buffer, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glObjectLabel(GL_SHADER, fragment_shader, -1, "Fragment Shader");
    glShaderSource(fragment_shader, 1, &f_sh_buffer, NULL);
    glCompileShader(fragment_shader);


    /* ======================================================== */
    GLint status = checkPipelineStatus(vertex_shader, fragment_shader);
    /* ======================================================== */

    program = glCreateProgram();
    glObjectLabel(GL_PROGRAM, program, -1, "Volumetric lighting");
    
    if (status) {
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        GLint link_status;
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);
        if (!link_status) {
            GLchar comp_info[1024];
            glGetProgramInfoLog(program, 1024, NULL, comp_info);

            fwrite(comp_info, 1024, 1, stdout);
        }
    }


    //GLuint pipeline;
    //glGenProgramPipelines(1, &pipeline);
    //glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, fragment_shader);
    //glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, vertex_shader);
    



    ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);

    AkDoc* doc;
    AkVisualScene* scene;
    AkCamera* camera;
    AkInstanceGeometry* geometry;
    AkNode *root, *node_ptr;

    std::string scene_path = "./res/ship_in_clouds/scene.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        std::cout << "Document couldn't be loaded\n";
    }else {
        std::cout << printCoordSys(doc->coordSys);
        std::cout << printInf(doc->inf, doc->unit);
        std::cout << "==============================================================================\n";
    }

    float* camera_mat = (float*)calloc(16, sizeof(float));
    float* camera_proj = (float*)calloc(16, sizeof(float));
    int8_t* raw_buffer;
    int buffer_size = 0;
    glm::mat4 Projection;

    if (doc->scene.visualScene) {
        scene = (AkVisualScene*) ak_instanceObject(doc->scene.visualScene);
        std::cout << "Visual Scene loaded\n";

        if(scene->name) std::cout << "Scene name: " << scene->name << std::endl;
        if (scene->lights)
            if (scene->lights->first) {
                AkLight* light = (AkLight*) ak_instanceObject(scene->lights->first->instance);
                if(light)
                std::cout << "Light name: " << light->name << std::endl;
            }
        if (scene->cameras)
            if (scene->cameras->first) {
                AkCamera* camera = (AkCamera*) ak_instanceObject(scene->cameras->first->instance);
                if(camera)
                std::cout << "Camera name: " << camera->name << std::endl;
            }
        ak_firstCamera(doc, &camera, camera_mat, camera_proj);
        if (camera) {
            std::cout << "Camera:" << camera->name << std::endl;
            Projection = glm::make_mat4x4(camera_proj);
        }else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);
        }
            
        for (int i = 0; i < 16; i++) {
            std::cout << camera_mat[i] << ", ";
            if (i % 4 == 3) std::cout << std::endl;
        }


        //AkMaterial* m = (AkMaterial*) doc->lib.materials->chld;
        //do {
        //    AkEffect* ef = (AkEffect*)ak_instanceObject(&m->effect->base);
        //    AkTechniqueFxCommon* tch = ef->profile->technique->common;
        //    m = (AkMaterial*)m->base.next;
        //}
        //while (m);


        //AkNode* node_ptr = (AkNode*) doc->lib.nodes->chld;
        AkNode* node_ptr = ak_instanceObjectNode(scene->node);
        AkNode* node = node_ptr;
        int j = 0;

        proccess_node(node); // pointer to pointer?
    }

    glm::mat4 Camera = glm::make_mat4x4(camera_mat);
    if (camera_mat) free(camera_mat);
    if (camera_proj) free(camera_proj);



    /* ======================================================== */

    mvp_location = glGetUniformLocation(program, "MVP");
    norm_location = glGetAttribLocation(program, "vNor");
    vpos_location = glGetAttribLocation(program, "vPos");
    tex_location = glGetAttribLocation(program, "vTex");


    glEnableVertexAttribArray(mvp_location);
    glEnableVertexAttribArray(vpos_location);
    if(norm_location != -1) glEnableVertexAttribArray(norm_location);
    if (tex_location != -1) glEnableVertexAttribArray(tex_location);
    

    int j;
    
    // What with images and libimages ??
    j = 0;
    FListItem* t = doc->lib.textures;
    if (t) {
        do {
            AkTexture* tex = (AkTexture*)t->data;
            textureViews.insert({ {tex, 0} });
            t = t->next;
        } while (t);
        for (auto& u : textureViews) {
            u.second = j++;
        }
    }


    j = 0;
    FListItem* b = (FListItem*)doc->lib.buffers;
    if (b) {
        do {
            AkBuffer* buf = (AkBuffer*)b->data;
            bufferViews.insert({ {buf, 0} });
            b = b->next;
        } while (b);
        for (auto& u : bufferViews) {
            u.second = j++;
        }
        print_map(bufferViews);
    }


    GLuint* buffers = (GLuint*) calloc(bufferViews.size(), sizeof(GLuint));
    glCreateBuffers(bufferViews.size(), buffers);
    for (auto &buffer : bufferViews) {
        unsigned int i = bufferViews[buffer.first];
        glNamedBufferData(buffers[i], ((AkBuffer*) buffer.first)->length, ((AkBuffer*) buffer.first)->data, GL_STATIC_DRAW);
    } 
    
    for (int i = 0; i < primitives.size(); i++) {
        
        formatAttribute(vpos_location, primitives[i].pos);
        if(norm_location != -1) formatAttribute(norm_location, primitives[i].nor);
        if (tex_location != -1) formatAttribute(tex_location, primitives[i].tex);
        //binding_point = i;
        //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");
    }
    //size_t l = primitives[0].pos->byteLength / primitives[0].pos->componentSize;
    //size_t o = primitives[0].pos->byteOffset;
    //for (size_t i = o; i < o+l; i++) {
    //    std::cout << ((float*) primitives[0].pos->buffer->data)[i] << ", ";
    //    if (i % 20 == 19) std::cout << std::endl;
    //}
    //

    glDepthFunc(GL_GEQUAL);
    //glEnable(GL_DEPTH_TEST);

    std::cout << "===================== Main loop ==============================================\n";
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < primitives.size(); i++) {
            float r = (float) 0.1 * panel_config.p6;
            float phi = panel_config.p7;//(float) 3.14 * panel_config.p7 / 180;
            float theta = panel_config.p8;//(float) 3.14 * panel_config.p8 / 180;

            glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
            eye = glm::vec3(eye.z, eye.y, eye.x);
            glm::vec3 north = glm::vec3(0., 1., 0.);
            float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
            if (corrected_theta > 3.14/2. && corrected_theta < 3.14 * 3./2.) {
                north = glm::vec3(0., -1., 0.);
            }
            
            glm::vec3 translate = glm::vec3(panel_config.p1 * 0.1, panel_config.p2 * 0.1, panel_config.p3 * 0.1);
            glm::vec3 rotate = glm::vec3(3.14 * panel_config.p4 / 180, 3.14 * panel_config.p5 / 180, 0.f);

            glUseProgram(program);
            //glBindProgramPipeline(pipeline);

            glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
            //glm::mat4 Projection = glm::perspectiveFov((float) 3.14*panel_config.fov/180, (float) width, (float) height, panel_config.near_plane, panel_config.far_plane);

            //glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), translate);
            //glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
            //glm::mat4 View = glm::rotate(ViewRotateX, rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 View = glm::make_mat4x4(primitives[i].transform);
            glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
            glm::mat4 MVP = Projection * LookAt * View * Model;
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));

            int j = bufferViews[primitives[i].pos->buffer];
            int binding_point = 0;
            glVertexAttribBinding(vpos_location, binding_point);
            glBindVertexBuffer(binding_point, buffers[j], primitives[i].pos->byteOffset, primitives[i].pos->componentBytes);
            
            if (norm_location != -1) {
                j = bufferViews[primitives[i].nor->buffer];
                binding_point = 1;
                glVertexAttribBinding(norm_location, binding_point);
                glBindVertexBuffer(binding_point, buffers[j], primitives[i].nor->byteOffset, primitives[i].nor->componentBytes);
            }

            if (tex_location != -1) {
                j = bufferViews[primitives[i].tex->buffer];
                binding_point = 2;
                glVertexAttribBinding(tex_location, binding_point);
                glBindVertexBuffer(binding_point, buffers[j], primitives[i].tex->byteOffset, primitives[i].tex->componentBytes);
            }
            GLuint* sampler = primitives[i].emi_sampler;
            GLuint* texture = primitives[i].emi_texture;
            if (sampler && texture) {
                glBindSampler(0, *sampler);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, *texture);
            }

            glDrawElements(GL_TRIANGLES, primitives[i].ind_size, GL_UNSIGNED_INT, primitives[i].ind);
        }
        

        

        //glBindProgramPipeline(0);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(io);
        drawRightPanel(io, panel_config);
        
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    free(v_sh_buffer);
    free(f_sh_buffer);

    for (auto& p : primitives) p.deleteTranforms();


    for (auto& p : primitives) {
        if (p.amb_sampler) {
            glDeleteSamplers(1, p.amb_sampler);
            //free(p.amb_sampler);
        }
        if (p.emi_sampler) {
            glDeleteSamplers(1, p.emi_sampler);
            //free(p.emi_sampler);
        }
        if (p.diff_sampler) {
            glDeleteSamplers(1, p.diff_sampler);
            //free(p.diff_sampler);
        }
        if (p.spec_sampler) {
            glDeleteSamplers(1, p.spec_sampler);
            //free(p.spec_sampler);
        }
        if (p.amb_texture) {
            glDeleteTextures(1, p.amb_texture);
            //free(p.amb_texture);
        }
        if (p.emi_texture) {
            glDeleteTextures(1, p.emi_texture);
            //free(p.emi_texture);
        }
        if (p.diff_texture) {
            glDeleteTextures(1, p.diff_texture);
           // free(p.diff_texture);
        }
        if (p.spec_texture) {
            glDeleteTextures(1, p.spec_texture);
            //free(p.spec_texture);
        }
    }
        
    
    //glDeleteProgramPipelines(1, &pipeline);

    //GLuint buffers[] = {vertex_buffer};

    glDeleteBuffers(bufferViews.size(), buffers);
    free(buffers);
    glDeleteVertexArrays(1, &vao);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glDeleteProgram(program);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    std::cout << "========== [GLFW]: Terminated ================================================\n";
    std::cout << "===================== Exit succeeded =========================================\n";
    return 0;
}